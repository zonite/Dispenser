#include <net/genetlink.h>
#include <linux/version.h>

#include "dispenser.h"

// https://github.com/phip1611/generic-netlink-user-kernel-rust
// https://stackoverflow.com/questions/60821210/how-to-send-and-receive-a-struct-through-netlink

//#define DISPENSER_GENL_OPS_LEN DISPENSER_GENL_CMD_COUNT
//#define DISPENSER_GENL_ATTR_LEN DISPENSER_GENL_ATTR_COUNT

/** Dispenser Netlink Generic: private prototypes **/
static int dispenser_genl_release(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_slot_status(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_col_status(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_unit_status(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_environment(struct sk_buff *sender_buffer, struct  genl_info *info);
//static int dispenser_genl_temperature_calibration(struct sk_buff *sender_buffer, struct  genl_info *info);
//static int dispenser_genl_pressure_calibration(struct sk_buff *sender_buffer, struct  genl_info *info);
//static int dispenser_genl_humidity_calibration(struct sk_buff *sender_buffer, struct  genl_info *info);
//static int dispenser_genl_dump(struct sk_buff *sender_buffer, struct  genl_info *info);

static int __dispenser_genl_post_slot_status(struct dispenser_slot_list *slot, struct  genl_info *info);
static int __dispenser_genl_post_col_status(struct dispenser_col_list *col, struct  genl_info *info);
static int __dispenser_genl_post_unit_status(struct  genl_info *info);
static int __dispenser_genl_post_environment(struct  genl_info *info);


//static const struct genl_ops dispenser_genl_ops[DISPENSER_GENL_CMD_MAX] = {
static const struct genl_ops dispenser_genl_ops[] = {
/*
{
	.cmd = DISPENSER_GENL_CMD_UNSPEC, //NOT needed
	.flags = 0,
	.internal_flags = 0,
	.doit = callback,
	.dumpit = callback_noallocation, //pre allocated buffer...
	.start = lock,
	.done = unlock,
	.validate = 0,
},
*/
{
	.cmd = DISPENSER_GENL_CMD_RELEASE,
	.doit = dispenser_genl_release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)
	.policy = dispenser_genl_policy
#endif
},
{
	.cmd = DISPENSER_GENL_CMD_SLOT_STATUS,
	.doit = dispenser_genl_slot_status,
},
{
	.cmd = DISPENSER_GENL_CMD_COL_STATUS,
	.doit = dispenser_genl_col_status,
},
{
	.cmd = DISPENSER_GENL_CMD_UNIT_STATUS,
	.doit = dispenser_genl_unit_status,
},
{
	.cmd = DISPENSER_GENL_CMD_ENVIRONMENT,
	.doit = dispenser_genl_environment,
},
/*
{
	.cmd = DISPENSER_GENL_CMD_TEMPERATURE_CALIBRATION,
	.doit = dispenser_genl_temperature_calibration,
},
{
	.cmd = DISPENSER_GENL_CMD_PRESSURE_CALIBRATION,
	.doit = dispenser_genl_pressure_calibration,
},
{
	.cmd = DISPENSER_GENL_CMD_HUMIDITY_CALIBRATION,
	.doit = dispenser_genl_humidity_calibration,
},
{
	.cmd = DISPENSER_GENL_CMD_DUMP,
	.dumpit = dispenser_genl_dump,
},
*/
};

static struct nla_policy const dispenser_genl_policy[DISPENSER_GENL_ATTR_COUNT] = {
	[DISPENSER_GENL_CMD_UNSPEC] = { .type = NLA_UNSPEC },
	[DISPENSER_GENL_MEM_COUNTER] = { .type = NLA_U32 }, //u32 attr
	[DISPENSER_GENL_RELEASE_COUNT] = { .type = NLA_S8 }, //s8 attr // negative num to force
	[DISPENSER_GENL_COL_NUM] = { .type = NLA_U8 }, //u8 attr
	[DISPENSER_GENL_SLOT_NUM] = { .type = NLA_U8 }, //u8 attr
	[DISPENSER_GENL_SLOT_STATUS] = { .type = NLA_U8 }, //bitfield up,down,release,full,+enum state (4bits) (settable)
	[DISPENSER_GENL_SLOT_FAILED_UP] = { .type = NLA_S32 }, //u32 attr
	[DISPENSER_GENL_SLOT_FAILED_DOWN] = { .type = NLA_S32 }, //u32 attr
	[DISPENSER_GENL_UNIT_STATUS] = { .type = NLA_U8 }, //bitfield door,power,night,light (night+light settable)
	[DISPENSER_GENL_TEMPERATURE] = { .type = NLA_S32 }, //s32 attr //temperature
	[DISPENSER_GENL_PRESSURE] = { .type = NLA_U32 }, //u32 attr //pressure
	[DISPENSER_GENL_HUMIDITY] = { .type = NLA_U32 }, //u32 attr //humidity
	[DISPENSER_GENL_CALIBRATION0] = { .type = NLA_U64 }, //calibration data u64
	[DISPENSER_GENL_CALIBRATION1] = { .type = NLA_S16 }, //calibration data s16
//	[DISPENSER_GENL_CALIBRATION] = { .type = NLA_U32 }, //calibration data
};

/** Generic Netlink Family Descriptor **/
static struct genl_family dispenser_genl_family = {
	.id = 0, //Not needed
	.hdrsize = 0, //Not needed
	.name = DISPENSER_GENL_NAME,
	.version = 1,
	.ops = dispenser_genl_ops,
	.n_ops = ARRAY_SIZE(dispenser_genl_ops),
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0)
	.policy = dispenser_genl_policy,
#endif
	.maxattr = DISPENSER_GENL_ATTR_MAX,
	.module = THIS_MODULE,
	.parallel_ops = 0, //Not needed
	.netnsok = 0, //Not needed
	.pre_doit = NULL, //Not needed
	.post_doit = NULL, //Not needed
};

/** Release a slot from dispenser **/
static int dispenser_genl_release(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	struct nlattr **attrs;
	struct dispenser_col_list *col;
	s8 count, force;
	u8 s, c;

	if (!info || !info->attrs) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	attrs = info->attrs;
	count = 1;
	force = 0;

	if (attrs[DISPENSER_GENL_RELEASE_COUNT]) {
		count = nla_get_s8(attrs[DISPENSER_GENL_RELEASE_COUNT]);
	}

	if (!count)
		return -EINVAL;

	if (count < 0) {
		force = 1;
		count = (~count) + 1; //one's complement
	}

	if (attrs[DISPENSER_GENL_COL_NUM]) {
		c = nla_get_u8(attrs[DISPENSER_GENL_COL_NUM]);
		col = dispenser_unit_get_column(c);

		if (!col) {
			return -EINVAL;
		}

		if (attrs[DISPENSER_GENL_SLOT_NUM]) {
			s = nla_get_u8(attrs[DISPENSER_GENL_SLOT_NUM]);
			return dispenser_unit_release(c, s); //release specific slot
		}

		dispenser_unit_release_column(col, count, force); //release from column
		return 0;
	}

	dispenser_unit_release_count(count, force); //release from unit

	return 0;
}

/** Get slot status request **/
static int dispenser_genl_slot_status(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	struct nlattr **attrs;
	u8 s, c, status = 0;
	struct dispenser_slot_list *slot;

	if (!info || !info->attrs) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	attrs = info->attrs;

	if (!attrs[DISPENSER_GENL_COL_NUM] || attrs[DISPENSER_GENL_SLOT_NUM] ) {
		printk("Error: no column or slot\n");
		return -EINVAL;
	}

	c = nla_get_u8(attrs[DISPENSER_GENL_COL_NUM]);
	s = nla_get_u8(attrs[DISPENSER_GENL_SLOT_NUM]);

	slot = dispenser_unit_get(c, s);
	if (!slot) {
		printk("Slot not found %u, %u\n", c, s);
		return -EINVAL;
	}

	if (attrs[DISPENSER_GENL_SLOT_STATUS]) {
		struct dispenser_mmap_slot new_state;
		unsigned char full;
		memcpy(&new_state, slot->state, sizeof (struct dispenser_mmap_slot));

		status = nla_get_u8(attrs[DISPENSER_GENL_SLOT_STATUS]);
		dispenser_unpack_slot_status(status, &new_state, &full);
		dispenser_unit_set_slot_state(slot, &new_state, &full);
		//dispenser_unpack_slot_status(status, slot->state, &slot->full);
	}

	if (attrs[DISPENSER_GENL_SLOT_FAILED_UP]) {
		dispenser_unit_set_slot_up_failed(slot, nla_get_s32(attrs[DISPENSER_GENL_SLOT_FAILED_UP]));
		//slot->state->up_failed = nla_get_s32(attrs[DISPENSER_GENL_SLOT_FAILED_UP]);
	}

	if (attrs[DISPENSER_GENL_SLOT_FAILED_DOWN]) {
		dispenser_unit_set_slot_down_failed(slot, nla_get_s32(attrs[DISPENSER_GENL_SLOT_FAILED_DOWN]));
		//slot->state->down_failed = nla_get_s32(attrs[DISPENSER_GENL_SLOT_FAILED_DOWN]);
	}

	return __dispenser_genl_post_slot_status(slot, info);
}

/** Post slot status as event **/
static int __dispenser_genl_post_slot_status(struct dispenser_slot_list *slot, struct  genl_info *info)
{
	struct sk_buff *reply_buffer;
	void *reply_header;
	//s32 up_failed, down_failed;
	u8 status = 0;
	static u32 seq = 0;
	int ret;

	if (!slot) {
		printk("Invalid slot %p\n", slot);
		return -EINVAL;
	}

	// Send the status:

	//bitfield up,down,release,full,+enum state (4bits) (settable)
	status = dispenser_pack_slot_status(slot->state, slot->full);

	reply_buffer = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (reply_buffer == NULL) {
		printk("Error: No memory in %s.\n", __func__);
		return -ENOMEM;
	}

	if (info) {
		reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_SLOT_STATUS);
	} else {
		//info == NULL --> broadcast!
		//reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_UNIT_STATUS);
		reply_header = genlmsg_put(reply_buffer, 0, seq++, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_SLOT_STATUS);
	}
	if (reply_header == NULL) {
		printk("Header memory error.\n");
		return -ENOMEM;
	}

	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_SLOT_STATUS, status);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	if (slot->state->state != UNKNOWN) {
		ret = nla_put_u32(reply_buffer, DISPENSER_GENL_SLOT_FAILED_UP, slot->state->up_failed);
		if (ret) {
			printk("Error adding failed up to message.\n");
			return -ret;
		}

		ret = nla_put_u32(reply_buffer, DISPENSER_GENL_SLOT_FAILED_DOWN, slot->state->down_failed);
		if (ret) {
			printk("Error adding failed down to message.\n");
			return -ret;
		}
	}

	ret = nla_put_u32(reply_buffer, DISPENSER_GENL_MEM_COUNTER, dispenser_unit_counter());
	if (ret) {
		printk("Error adding counter to message.\n");
		return -ret;
	}

	genlmsg_end(reply_buffer, reply_header);

	if (info) {
		return genlmsg_reply(reply_buffer, info);
	} else {
		return genlmsg_multicast(&dispenser_genl_family, reply_buffer, 0, DISPENSER_GENL_GROUP, 0);
	}
}

/** Set col status request **/
static int dispenser_genl_col_status(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	struct nlattr **attrs;
	struct dispenser_col_list *col;
	u8 c;

	if (!info || !info->attrs) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	if (!pDispenser_mmap) {
		return -EINVAL;
	}

	attrs = info->attrs;

	if (!attrs[DISPENSER_GENL_COL_NUM]) {
		printk("Error: no column\n");
		return -EINVAL;
	}

	c = nla_get_u8(attrs[DISPENSER_GENL_COL_NUM]);

	col = dispenser_unit_get_column(c);
	if (!col) {
		printk("Column not found %u\n", c);
		return -EINVAL;
	}

	return __dispenser_genl_post_col_status(col, info);
}

/** Post col status reply **/
static int __dispenser_genl_post_col_status(struct dispenser_col_list *col, struct  genl_info *info)
{
	struct sk_buff *reply_buffer;
	void *reply_header;
	//s32 up_failed, down_failed;
	static u32 seq = 0;
	int ret;

	if (!col) {
		printk("Invalid column %p\n", col);
		return -EINVAL;
	}

	reply_buffer = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (reply_buffer == NULL) {
		printk("Error: No memory in %s.\n", __func__);
		return -ENOMEM;
	}

	if (info) {
		reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_COL_STATUS);
	} else {
		//info == NULL --> broadcast!
		//reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_UNIT_STATUS);
		reply_header = genlmsg_put(reply_buffer, 0, seq++, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_COL_STATUS);
	}
	if (reply_header == NULL) {
		printk("Header memory error.\n");
		return -ENOMEM;
	}

	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_COL_NUM, col->col_id);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

/* Not implemented
	ret = nla_put_string(reply_buffer, DISPENSER_GENL_COL_NAME, col->col_name);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}
*/
	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_SLOT_NUM, col->slot_count);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u32(reply_buffer, DISPENSER_GENL_MEM_COUNTER, dispenser_unit_counter());
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	genlmsg_end(reply_buffer, reply_header);

	if (info) {
		return genlmsg_reply(reply_buffer, info);
	} else {
		return genlmsg_multicast(&dispenser_genl_family, reply_buffer, 0, DISPENSER_GENL_GROUP, 0);
	}
}

/** Set unit status request **/
static int dispenser_genl_unit_status(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	struct nlattr **attrs;
	struct dispenser_mmap_unit *unit;
	u8 status = 0, ret = 0;

	if (!info || !info->attrs) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	if (!pDispenser_mmap) {
		return -EINVAL;
	}
	unit = &pDispenser_mmap->unit;

	attrs = info->attrs;

	if (attrs[DISPENSER_GENL_UNIT_STATUS]) {
		struct dispenser_mmap_unit new_state;
		memcpy(&new_state, unit, sizeof(struct dispenser_mmap_unit));

		status = nla_get_u8(attrs[DISPENSER_GENL_UNIT_STATUS]);
		dispenser_unpack_unit_status(status, &new_state);

		dispenser_unit_set_state(&new_state);
	}

	if (attrs[DISPENSER_GENL_INITIALIZED]) {
		ret = nla_get_u8(attrs[DISPENSER_GENL_INITIALIZED]);
		unit->initialized = ret;
	}

	return __dispenser_genl_post_unit_status(info);
}

/** Get unit status request **/
static int __dispenser_genl_post_unit_status(struct genl_info *info)
{
	struct sk_buff *reply_buffer;
	struct dispenser_mmap_unit *unit;
	void *reply_header;
	u8 status = 0;
	static u32 seq = 0;
	int ret;

	if (!pDispenser_mmap) {
		return -EINVAL;
	}
	unit = &pDispenser_mmap->unit;


	// Send the status:

	//bitfield door,power,night,light (night+light settable)
	status = dispenser_pack_unit_status(unit);

	reply_buffer = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (reply_buffer == NULL) {
		printk("Error: No memory in %s.\n", __func__);
		return -ENOMEM;
	}

	if (info) {
		reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_UNIT_STATUS);
	} else {
		//info == NULL --> broadcast!
		//reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_UNIT_STATUS);
		reply_header = genlmsg_put(reply_buffer, 0, seq++, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_UNIT_STATUS);
	}
	if (reply_header == NULL) {
		printk("Header memory error.\n");
		return -ENOMEM;
	}

	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_UNIT_STATUS, status); //Door, charging, night, light
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_COL_NUM, unit->ncols); //num columns
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_SLOT_NUM, unit->nslots); //num slots
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u8(reply_buffer, DISPENSER_GENL_INITIALIZED, unit->initialized);
	if (ret) {
		printk("Error adding counter to message.\n");
		return -ret;
	}

	ret = nla_put_u32(reply_buffer, DISPENSER_GENL_MEM_COUNTER, dispenser_unit_counter());
	if (ret) {
		printk("Error adding counter to message.\n");
		return -ret;
	}

	genlmsg_end(reply_buffer, reply_header);

	if (info) {
		return genlmsg_reply(reply_buffer, info);
	} else {
		return genlmsg_multicast(&dispenser_genl_family, reply_buffer, 0, DISPENSER_GENL_GROUP, 0);
	}
}

/** Get unit environment request **/
static int dispenser_genl_environment(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	if (!info || !info->attrs) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	return __dispenser_genl_post_environment(info);
}

static int __dispenser_genl_post_environment(struct  genl_info *info)
{
	struct sk_buff *reply_buffer;
	void *reply_header;
	static u32 seq = 0;
	int ret;
	struct dispenser_mmap_unit *unit;

	// Send the status:
	if (!pDispenser_mmap) {
		return -EAGAIN;
	}
	unit = &pDispenser_mmap->unit;

	reply_buffer = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (reply_buffer == NULL) {
		printk("Error: No memory in %s.\n", __func__);
		return -ENOMEM;
	}

	if (info) {
		reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_ENVIRONMENT);
	} else {
		//info == NULL --> broadcast!
		reply_header = genlmsg_put(reply_buffer, 0, seq++, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_ENVIRONMENT);
		//reply_header = genlmsg_put(reply_buffer, info->snd_portid, info->snd_seq + 1, &dispenser_genl_family, 0, DISPENSER_GENL_CMD_ENVIRONMENT);
	}

	if (reply_header == NULL) {
		printk("Header memory error.\n");
		return -ENOMEM;
	}

	ret = nla_put_s32(reply_buffer, DISPENSER_GENL_TEMPERATURE, pDispenser_mmap->unit.temperature);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u32(reply_buffer, DISPENSER_GENL_PRESSURE, pDispenser_mmap->unit.pressure);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u32(reply_buffer, DISPENSER_GENL_HUMIDITY, pDispenser_mmap->unit.humidity);
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	ret = nla_put_u32(reply_buffer, DISPENSER_GENL_MEM_COUNTER, dispenser_unit_counter());
	if (ret) {
		printk("Error adding status to message.\n");
		return -ret;
	}

	genlmsg_end(reply_buffer, reply_header);

	if (info) {
		return genlmsg_reply(reply_buffer, info);
	} else {
		return genlmsg_multicast(&dispenser_genl_family, reply_buffer, 0, DISPENSER_GENL_GROUP, 0);
	}
}


/*
static int dispenser_genl_calibration(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	if (!info) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

}
*/

/*
static int dispenser_genl_dump(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	if (!info) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

}


static int dispenser_genl_doit(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	struct nlattr *attr;
	struct sk_buff *reply_buffer;
	int ret;
	void *msg;
	char *recv_msg;

	if (!info) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	attr = info->attrs[MSG];

	if (!attr) {
		printk("Error: null info on attrs.\n");
		return -EINVAL;
	}

	recv_msg = (char *) nla_data(attr);
	if (!recv_msg) {
		printk("Error: no data received.\n");
	} else {
		printk("Received: %s\n", recv_msg);
	}

	reply_buffer = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!reply_buffer) {
		printk("Error: no memory\n");
		return -ENOMEM;
	}

	msg = genlmsg_put(reply_buffer,
			  info->snd_portid,
			  info->snd_seq + 1,
			  &dispenser_genl_family,
			  0,
			  CMD
			  );
	if (!msg) {
		printk("Error: no memory\n");
		return -ENOMEM;
	}

	ret = nla_put_string(reply_buffer, CMD, recv_msg);
	if (ret) {
		printk("Error putting message\n");
		return -ret;
	}

	genlmsg_end(reply_buffer, msg);

	ret = genlmsg_reply(reply_buffer, info);

	if (ret) {
		printk("Error: reply not 0\n");
		return -ret;
	}

	return 0;
}
*/

static int dispenser_genl_init(void)
{
	if (!genl_register_family(&dispenser_genl_family)) {
		printk("Dispenser: successful Generic Netlink Registration.\n");
	} else {
		printk("Dispenser: Error Generic Netlink Registration.\n");
		//printk("Returned %i\n", genl_register_family(&dispenser_genl_family));
		return -1;
	}

	return 0;
}

static void dispenser_genl_exit(void)
{
	if (!genl_unregister_family(&dispenser_genl_family)) {
		printk("Dispenser: successful Generic Netlink Unregistration.\n");
	} else {
		printk("Dispenser: Error Generic Netlink Unregistration.\n");
		return;
	}

}

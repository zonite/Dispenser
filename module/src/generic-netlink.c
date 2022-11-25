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
static int dispenser_genl_unit_status(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_environment(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_calibration(struct sk_buff *sender_buffer, struct  genl_info *info);
static int dispenser_genl_dump(struct sk_buff *sender_buffer, struct  genl_info *info);


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
	.cmd = DISPENSER_GENL_CMD_UNIT_STATUS,
	.doit = dispenser_genl_unit_status,
},
{
	.cmd = DISPENSER_GENL_CMD_ENVIRONMENT,
	.doit = dispenser_genl_environment,
},
{
	.cmd = DISPENSER_GENL_CMD_CALIBRATION,
	.doit = dispenser_genl_calibration,
},
{
	.cmd = DISPENSER_GENL_CMD_DUMP,
	.dumpit = dispenser_genl_dump,
},
};

static struct nla_policy const dispenser_genl_policy[DISPENSER_GENL_ATTR_COUNT] = {
	[DISPENSER_GENL_CMD_UNSPEC] = { .type = NLA_UNSPEC },
	[DISPENSER_GENL_MEM_COUNTER] = { .type = NLA_U32 }, //u32 attr
	[DISPENSER_GENL_RELEASE_COUNT] = { .type = NLA_S8 }, //s8 attr // negative num to force
	[DISPENSER_GENL_COL_NUM] = { .type = NLA_U8 }, //u8 attr
	[DISPENSER_GENL_SLOT_NUM] = { .type = NLA_U8 }, //u8 attr
	[DISPENSER_GENL_SLOT_STATUS] = { .type = NLA_U8 }, //bitfield up,down,release,+enum state (5bits) (settable)
	[DISPENSER_GENL_SLOT_FAILED_UP] = { .type = NLA_S32 }, //u32 attr
	[DISPENSER_GENL_SLOT_FAILED_DOWN] = { .type = NLA_S32 }, //u32 attr
	[DISPENSER_GENL_UNIT_STATUS] = { .type = NLA_U8 }, //bitfield door,power,night,light (night+light settable)
	[DISPENSER_GENL_TEMPERATURE] = { .type = NLA_U32 }, //u32 attr //raw temperature
	[DISPENSER_GENL_PRESSURE] = { .type = NLA_U32 }, //u32 attr //raw pressure
	[DISPENSER_GENL_HUMIDITY] = { .type = NLA_U32 }, //u32 attr //raw humidity
	[DISPENSER_GENL_CALIBRATION] = { .type = NLA_U32 }, //calibration data
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
		count = (~count) + 1;
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

static int dispenser_genl_slot_status(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	struct nlattr **attrs;
	s32 up_failed, down_failed;
	u8 s, c, status;

	if (!info || !info->attrs) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

	if (!attrs[DISPENSER_GENL_COL_NUM] || attrs[DISPENSER_GENL_SLOT_NUM] ) {
		printk("Error: no column or slot\n");
		return -EINVAL;
	}

	c = nla_get_u8(attrs[DISPENSER_GENL_COL_NUM]);
	s = nla_get_u8(attrs[DISPENSER_GENL_SLOT_NUM]);
}

static int dispenser_genl_unit_status(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	if (!info) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

}

static int dispenser_genl_environment(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	if (!info) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

}

static int dispenser_genl_calibration(struct sk_buff *sender_buffer, struct  genl_info *info)
{
	if (!info) {
		printk("Error: Generic Netlink null info.\n");
		return -EINVAL;
	}

}

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

static int dispenser_genl_init(void)
{
	if (genl_register_family(&dispenser_genl_family)) {
		printk("Dispenser: successful Generic Netlink Registration.\n");
	} else {
		printk("Dispenser: Error Generic Netlink Registration.\n");
		return -1;
	}

	return 0;
}

static void dispenser_genl_exit(void)
{
	if (genl_unregister_family(&dispenser_genl_family)) {
		printk("Dispenser: successful Generic Netlink Unregistration.\n");
	} else {
		printk("Dispenser: Error Generic Netlink Unregistration.\n");
		return;
	}

}

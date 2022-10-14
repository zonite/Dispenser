#include <net/genetlink.h>

#include "dispenser.h"

static struct genl_ops dispenser_genl_ops[DISPENSER_GENL_OPS_LEN] = {
{
	.cmd = ,
	.flags = 0,
	.internal_flags = 0,
	.doit = callback,
	.dumpit = callback_noallocation,
	.start = lock,
	.done = unlock,
	.validate = 0,
},
{
	.cmd = ,
	.flags = 0,
	.internal_flags = 0,
	.doit = callback,
	.dumpit = callback_noallocation,
	.start = lock,
	.done = unlock,
	.validate = 0,

}
};

static struct nla_policy dispenser_genl_policy[] = {
	[CMD] = { .type = NLA_UNSPEC },
	[CMD] = { .type = NLA_NUL_STRING },
};

static struct genl_family dispenser_genl_family = {
	.id = 0,
	.hdrsize = 0,
	.name = DISPENSER_GENL_NAME,
	.version = 1,
	.ops = dispenser_genl_ops,
	.n_ops = DISPENSER_GENL_OPS_LEN,
	.policy = dispenser_genl_policy,
	.maxattr = ,
	.module = THIS_MODULE,
	.parallel_ops = 0,
	.netnsok = 0,
	.pre_doit = NULL,
	.post_doit = NULL,
};

int dispenser_genl_doit(struct sk_buff *sender_buffer, struct  genl_info *info)
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

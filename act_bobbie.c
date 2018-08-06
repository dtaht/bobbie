/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* 
 * Copyright 2018 Michael D Taht
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <net/netlink.h>
#include <net/pkt_sched.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/inet_ecn.h>

#include "tcl_bobbie.h"
#include "tc_bobbie.h"

static unsigned int bobbie_net_id;
static struct tc_action_ops act_bobbie_ops;

static int tcf_bobbie(struct sk_buff *skb, const struct tc_action *a,
		       struct tcf_result *res)
{
	struct tcf_bobbie *d = to_bobbie(a);
	struct tcf_bobbie_params *params;
	int action;

	tcf_lastuse_update(&d->tcf_tm);
	bstats_cpu_update(this_cpu_ptr(d->common.cpu_bstats), skb);

	rcu_read_lock(); // But, er, we want to write some local state
	params = rcu_dereference(d->params);
        action = 0;
	//	action = READ_ONCE(d->tcf_action); // FIXME always shot

	// start at thinking about how to collect these stats
#if 0
	{
	s64 delta, ideal, actual;
	s64 now = ktime_get_ns();
	if (!skb->tstamp) skb->tstamp=now;
	if (!skb->hash) skb->hash = skb_get_hash(skb);
	delta = now - d->last;
	ideal = delta * d->time_per_byte; // fixme, scale these
	actual = delta - skb->len * d->time_per_byte;
	if (d->over_start * 4 * 1000000 * NSEC_PER_SEC > now) ; // (re) start the drop schedule;
	}
#endif
	if (params->flags & BOBBIE_F_ECN && INET_ECN_set_ce(skb)) { // Are we allowed to modify the header in flight?)
	  action = 0; // log stat somewhere
	}

unlock:
	rcu_read_unlock();
	return action;
err:
	qstats_drop_inc(this_cpu_ptr(d->common.cpu_qstats));
	action = TC_ACT_SHOT;
	goto unlock;
}

static const struct nla_policy bobbie_policy[TCA_BOBBIE_MAX + 1] = {
	[TCA_BOBBIE_PARMS]	= { .len = sizeof(struct tc_bobbie) },
	[TCA_BOBBIE_RATE]	= { .len = sizeof(u64) },
	[TCA_BOBBIE_FLAGS]	= { .len = sizeof(u32) },
	[TCA_BOBBIE_ECN]	= { .len = sizeof(u32) },
};

static int tcf_bobbie_init(struct net *net, struct nlattr *nla,
			    struct nlattr *est, struct tc_action **a,
			    int ovr, int bind, bool rtnl_held,
			    struct netlink_ext_ack *extack)
{
	struct tc_action_net *tn = net_generic(net, bobbie_net_id);
	struct tcf_bobbie_params *params_old, *params_new;
	struct nlattr *tb[TCA_BOBBIE_MAX + 1];
	struct tc_bobbie *parm;
	struct tcf_bobbie *d;
	u32 flags = 0;
	bool exists = false;
	int ret = 0, err;

	if (nla == NULL)
		return -EINVAL;

	err = nla_parse_nested(tb, TCA_BOBBIE_MAX, nla, bobbie_policy, NULL);
	if (err < 0)
		return err;

	if (tb[TCA_BOBBIE_PARMS] == NULL)
		return -EINVAL;

	if (tb[TCA_BOBBIE_ECN] != NULL) {
		flags |= BOBBIE_F_ECN;
	}

	parm = nla_data(tb[TCA_BOBBIE_PARMS]);

	err = tcf_idr_check_alloc(tn, &parm->index, a, bind);
	if (err < 0)
		return err;
	exists = err;
	if (exists && bind)
		return 0;

	if (!flags) {
		if (exists)
			tcf_idr_release(*a, bind);
		else
			tcf_idr_cleanup(tn, parm->index);
		return -EINVAL;
	}

	if (!exists) {
		ret = tcf_idr_create(tn, parm->index, est, a,
				     &act_bobbie_ops, bind, true);
		if (ret) {
			tcf_idr_cleanup(tn, parm->index);
			return ret;
		}

		d = to_bobbie(*a);
		ret = ACT_P_CREATED;
	} else {
		d = to_bobbie(*a);
		if (!ovr) {
			tcf_idr_release(*a, bind);
			return -EEXIST;
		}
	}

	ASSERT_RTNL();

	params_new = kzalloc(sizeof(*params_new), GFP_KERNEL);
	if (unlikely(!params_new)) {
		if (ret == ACT_P_CREATED)
			tcf_idr_release(*a, bind);
		return -ENOMEM;
	}

	params_new->flags = flags;
	if (flags & BOBBIE_F_ECN)
	  params_new->flags = 1; // FIXME

	d->tcf_action = parm->action;
	params_old = rtnl_dereference(d->params);
	rcu_assign_pointer(d->params, params_new);
	if (params_old)
		kfree_rcu(params_old, rcu);

	if (ret == ACT_P_CREATED)
		tcf_idr_insert(tn, *a);
	return ret;
}

static int tcf_bobbie_dump(struct sk_buff *skb, struct tc_action *a,
			    int bind, int ref)
{
	unsigned char *b = skb_tail_pointer(skb);
	struct tcf_bobbie *d = to_bobbie(a);
	struct tcf_bobbie_params *params;
	struct tc_bobbie opt = {
		.index   = d->tcf_index,
		.refcnt  = refcount_read(&d->tcf_refcnt) - ref,
		.bindcnt = atomic_read(&d->tcf_bindcnt) - bind,
		.action  = d->tcf_action,
	};
	struct tcf_t t;

	params = rtnl_dereference(d->params);

	if (nla_put(skb, TCA_BOBBIE_PARMS, sizeof(opt), &opt))
		goto nla_put_failure;
	if ((params->flags & BOBBIE_F_ECN) &&
	    nla_put_u32(skb, TCA_BOBBIE_ECN, 1))
		goto nla_put_failure;
	tcf_tm_dump(&t, &d->tcf_tm);
	if (nla_put_64bit(skb, TCA_BOBBIE_TM, sizeof(t), &t, TCA_BOBBIE_PAD))
		goto nla_put_failure;
	return skb->len;

nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static void tcf_bobbie_cleanup(struct tc_action *a)
{
	struct tcf_bobbie *d = to_bobbie(a);
	struct tcf_bobbie_params *params;

	params = rcu_dereference_protected(d->params, 1);
	if (params)
		kfree_rcu(params, rcu);
}

static int tcf_bobbie_walker(struct net *net, struct sk_buff *skb,
			      struct netlink_callback *cb, int type,
			      const struct tc_action_ops *ops,
			      struct netlink_ext_ack *extack)
{
	struct tc_action_net *tn = net_generic(net, bobbie_net_id);

	return tcf_generic_walker(tn, skb, cb, type, ops, extack);
}

static int tcf_bobbie_search(struct net *net, struct tc_action **a, u32 index,
			      struct netlink_ext_ack *extack)
{
	struct tc_action_net *tn = net_generic(net, bobbie_net_id);

	return tcf_idr_search(tn, a, index);
}

static int tcf_bobbie_delete(struct net *net, u32 index)
{
	struct tc_action_net *tn = net_generic(net, bobbie_net_id);

	return tcf_idr_delete_index(tn, index);
}

static struct tc_action_ops act_bobbie_ops = {
  .kind		=	"policer", // ?
  .type		=	TCA_ACT_BOBBIE, // ?
	.owner		=	THIS_MODULE,
	.act		=	tcf_bobbie,
	.dump		=	tcf_bobbie_dump,
	.init		=	tcf_bobbie_init,
	.cleanup	=	tcf_bobbie_cleanup,
	.walk		=	tcf_bobbie_walker,
	.lookup		=	tcf_bobbie_search,
	.delete		=	tcf_bobbie_delete,
	.size		=	sizeof(struct tcf_bobbie),
};

static __net_init int bobbie_init_net(struct net *net)
{
	struct tc_action_net *tn = net_generic(net, bobbie_net_id);

	return tc_action_net_init(tn, &act_bobbie_ops);
}

static void __net_exit bobbie_exit_net(struct list_head *net_list)
{
	tc_action_net_exit(net_list, bobbie_net_id);
}

static struct pernet_operations bobbie_net_ops = {
	.init = bobbie_init_net,
	.exit_batch = bobbie_exit_net,
	.id   = &bobbie_net_id,
	.size = sizeof(struct tc_action_net),
};

MODULE_AUTHOR("Dave Taht");
MODULE_DESCRIPTION("Bobbie: a modernized policer");
MODULE_LICENSE("GPL");

static int __init bobbie_init_module(void)
{
	return tcf_register_action(&act_bobbie_ops, &bobbie_net_ops);
}

static void __exit bobbie_cleanup_module(void)
{
	tcf_unregister_action(&act_bobbie_ops, &bobbie_net_ops);
}

module_init(bobbie_init_module);
module_exit(bobbie_cleanup_module);

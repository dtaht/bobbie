// SPDX

#ifndef __NET_TC_BOBBIE_H
#define __NET_TC_BOBBIE_H

#include <net/act_api.h>
#include "tcl_bobbie.h"

struct tcf_bobbie_params {
	u32 flags;
	struct rcu_head rcu;
};

struct tcf_bobbie {
	struct tc_action common;
	struct tcf_bobbie_params __rcu *params;
};

#define to_bobbie(a) ((struct tcf_bobbie *)a)

#endif /* __NET_TC_BOBBIE_H */

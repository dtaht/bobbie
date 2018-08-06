/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

#ifndef __LINUX_TC_BOBBIE_H
#define __LINUX_TC_BOBBIE_H

#include <linux/pkt_cls.h>

#define TCA_ACT_BOBBIE 12

#define BOBBIE_F_ECN			0x1

struct tc_bobbie {
	tc_gen;
};

enum {
	TCA_BOBBIE_UNSPEC,
	TCA_BOBBIE_RATE,
	TCA_BOBBIE_TM,
	TCA_BOBBIE_PAD,
	TCA_BOBBIE_ECN,
	TCA_BOBBIE_FLAGS,
	TCA_BOBBIE_PARMS,
	__TCA_BOBBIE_MAX
};
#define TCA_BOBBIE_MAX (__TCA_BOBBIE_MAX - 1)

#endif

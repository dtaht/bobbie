obj-m := act_police.o
KERNEL_VERSION := $(shell uname -r)
IDIR := /lib/modules/$(KERNEL_VERSION)/kernel/net/sched/
KDIR := /lib/modules/$(KERNEL_VERSION)/build
PWD := $(shell pwd)
VERSION := $(shell git rev-parse HEAD 2>/dev/null)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x$(VERSION)" CFLAGS_MODULE="-DCAKE_VERSION=\\\"$(VERSION)\\\"")

install:
	install -v -m 644 act_police.ko $(IDIR)
	depmod "$(KERNEL_VERSION)"
	[ "$(KERNEL_VERSION)" != `uname -r` ] || modprobe act_police

clean:
	rm -rf Module.markers modules.order Module.symvers act_police.ko act_police.mod.c act_police.mod.o act_police.o

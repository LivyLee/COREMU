### Makefile --- 

## Author: wu.andrew.xi@gmail.com
## Version: $Id: Makefile,v 0.0 2009/01/10 07:08:39 wuxi Exp $

# This makefile system follows the structuring conventions
# recommended by Peter Miller in his excellent paper:
#
#       Recursive Make Considered Harmful
#       http://aegis.sourceforge.net/auug97.pdf

# Make sure that 'all' is the first target
all:

include config.mk
include $(addsuffix /module.mk, $(modules))

objects += $(patsubst %.c, $(OBJDIR)/%.o, $(foreach dir, $(modules), $(wildcard $(dir)/*.c)))

all: $(objects) $(programs) $(archive) qemu qemu-install

install: qemu-install

coremu: $(objects) $(programs)
	@echo $LDFLAGS

LIBOBJS =  $(OBJDIR)/main/coremu_core.o
LIBOBJS += $(OBJDIR)/main/coremu_bus_lock.o
LIBOBJS += $(OBJDIR)/main/coremu_hw.o
LIBOBJS += $(OBJDIR)/main/coremu_hw_backend.o
LIBOBJS += $(OBJDIR)/main/coremu_hw_frontend.o
LIBOBJS += $(OBJDIR)/main/coremu_sched.o
LIBOBJS += $(OBJDIR)/main/coremu_timer.o
LIBOBJS += $(OBJDIR)/main/coremu_intr.o
LIBOBJS += $(OBJDIR)/main/ms_queue.o
LIBOBJS += $(OBJDIR)/main/timevar.o
LIBOBJS += $(OBJDIR)/main/tools.o

# No longer used. We do this in configure.
#header: $(OBJDIR)/qemu/config-host.h $(OBJDIR)/qemu/arm-softmmu/config-target.h
#$(OBJDIR)/qemu/config-host.h:
	#$(MAKE) -C $(COREMUDIR)/obj/qemu/ config-host.h
#$(OBJDIR)/qemu/arm-softmmu/config-target.h:
	#$(MAKE) -C $(COREMUDIR)/obj/qemu/arm-softmmu config-target.h

$(OBJDIR)/libcoremu.a: $(LIBOBJS)
	$(call quiet-command,rm -f $@ && $(AR) rcs $@ $^,"  AR    $(TARGET_DIR)$@")
qemu:
	@echo "---- [coremu]: build qemu ----"
	$(MAKE) -C $(COREMUDIR)/obj/qemu

qemu-install:
	$(MAKE) -C $(COREMUDIR)/obj/qemu install

bios-install:
	cp $(COREMUDIR)/coremubios/seabios/out/bios.bin $(COREMUDIR)/bin/qemu/share/qemu/seabios.bin

clean:
	$(RM) -rf $(addprefix obj/, $(modules))  $(addprefix bin/, $(modules))

distclean:
	$(RM) -rf obj/ bin/

.PHONY: distclean clean all qemu qemu-install install coremu

include rules.mk


bindir    := $(BINDIR)/utils
#programs  += $(bindir)/kill $(bindir)/monitor
programs  += $(bindir)/watch-test

UTILS_LINK := -lcurses

$(bindir)/watch-test:  $(OBJDIR)/utils/watch-test.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) $(UTILS_LINK) -o $@ $^, "  LINK    $@")


$(bindir)/kill:  $(OBJDIR)/utils/kill.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) $(LDFLAGS) $(UTILS_LINK) -o $@ $^, "  LINK    $@")

$(bindir)/monitor:  $(OBJDIR)/utils/monitor.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) $(LDFLAGS) $(UTILS_LINK) -o $@ $^, "  LINK    $@")

bindir    := $(BINDIR)/utils
#programs  += $(bindir)/kill $(bindir)/monitor
programs  += $(bindir)/watch-test $(bindir)/profile-tool $(bindir)/monitor

UTILS_LINK := -lcurses

$(bindir)/watch-test:  $(OBJDIR)/utils/watch-test.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) $(UTILS_LINK) -o $@ $^, "  LINK    $@")

$(bindir)/profile-tool: $(OBJDIR)/utils/profile-tool.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) $(LDFLAGS) $(UTILS_LINK) -o $@ $^, "  LINK    $@")

$(bindir)/monitor:  $(OBJDIR)/utils/monitor.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) $(LDFLAGS) $(UTILS_LINK) -o $@ $^, "  LINK    $@")

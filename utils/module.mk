bindir    := $(BINDIR)/utils
#programs  += $(bindir)/kill $(bindir)/monitor
programs  += $(bindir)/watch-test $(bindir)/profile-tool $(bindir)/monitor

$(bindir)/watch-test:  $(OBJDIR)/utils/watch-test.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) -o $@ $^, "  LINK    $@")

$(bindir)/profile-tool: $(OBJDIR)/utils/profile-tool.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) -o $@ $^, "  LINK    $@")

$(bindir)/monitor:  $(OBJDIR)/utils/monitor.o
	@mkdir -p $(@D)
	$(call quiet-command, $(LD) -lcurses -o $@ $^, "  LINK    $@")

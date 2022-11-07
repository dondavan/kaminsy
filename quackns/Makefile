MACH := quackns
SNAP := $(MACH)-0


.PHONY: on off recycle save

on:
	@vagrant up

off:
	@vagrant halt

recycle: off
	@sleep 2 && vagrant snapshot restore $(MACH) $(SNAP)

save:
	@vagrant snapshot save $(MACH) $(SNAP)

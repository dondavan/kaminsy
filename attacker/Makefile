MACH := mooncake
SNAP := $(MACH)-0

REM_IP := 192.168.10.30
DOMAIN := www.google.com
# Command to check if remote recursive resolver is running.
DIG_CMD := vagrant ssh -c 'time dig @$(REM_IP) +noall +answer $(DOMAIN)'
CHK_CMD := $(DIG_CMD) 2> /dev/null | grep -cF '$(DOMAIN)'


.PHONY: on off recycle save

on:
	@vagrant up

off:
	@vagrant halt

check:
	@if test `$(CHK_CMD)` -eq 1; then \
		echo 'OK';                    \
	else                              \
	 	echo 'FAIL';                  \
	fi

recycle: off
	@sleep 2 && vagrant snapshot restore $(MACH) $(SNAP)

save:
	@vagrant snapshot save $(MACH) $(SNAP)

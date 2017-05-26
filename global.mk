#
# Add macros and definitions which shall be available for all Makefiles
# This might be added to VDR main directory in the future

# build mode (0 - non-verbose, 1 - verbose)
VERBOSE ?= 0

# Desplay percentage (0 - no percentage, 1 - print xxx% (not 100% accurate!))
#WITH_PERCENT ?= 0
# does not work currently
override WITH_PERCENT := 0

# pretty print macros

ifeq ($(WITH_PERCENT),1)
  ifndef ECHO
	I := i
	TARGET_COUNTER = $(words $(I)) $(eval I += i)
	TOTAL_TARGETS := $(shell $(MAKE) $(MAKECMDGOALS) --dry-run --file=$(firstword $(MAKEFILE_LIST)) \
				     --no-print-directory --no-builtin-rules --no-builtin-variables ECHO="COUNTTHIS" | grep -c "COUNTTHIS")
	ECHO = echo "[$(shell expr "  $(shell echo $$((${TARGET_COUNTER} * 100 / ${TOTAL_TARGETS})))" : '.*\(...\)$$')%]"
  endif
else
	ECHO := echo
endif

ifeq ($(VERBOSE),0)
	override Q := @
	PRETTY_PRINT = @$(ECHO) $(1)
	AR_NUL := > /dev/null 2>&1
else
	override Q :=
	PRETTY_PRINT :=
	AR_NUL :=
endif


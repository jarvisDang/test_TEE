ifneq ($O,)
	out-dir := $O
else
	# If no build folder has been specified, then create all build files in
	# the current directory under a folder named out.
	out-dir := $(CURDIR)/out
endif

-include $(TA_DEV_KIT_DIR)/host_include/conf.mk

ifneq ($V,1)
	q := @
else
	q :=
endif

.PHONY: all
all: ta ca

.PHONY: ta ca
ta:
	$(q)$(MAKE) -C ta -f makefile.optee CROSS_COMPILE="$(CROSS_COMPILE_TA)" \
			  q=$(q) \
			  O=$(out-dir)/ta \
			  $@

ca:
	$(q)$(MAKE) -C ca CROSS_COMPILE="$(CROSS_COMPILE_CA)" \
			  q=$(q) \
			  O=$(out-dir)/ca \
			  $@

.PHONY: clean
clean:
	$(q)$(MAKE) -C ta -f makefile.optee O=$(out-dir)/ta q=$(q) $@
	$(q)$(MAKE) -C ca O=$(out-dir)/ca q=$(q) $@


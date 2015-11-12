XSENS_INSTALL_PREFIX?=$(shell dirname $$(dirname $$(pwd)))
XSENS_PUBLIC_SOURCE?=$(XSENS_INSTALL_PREFIX)/public

IMU2SIMULINK_PREREQS:=src/xstypes src/include src/xcommunication
COMMON_TARGETS=src
TARGETS=$(COMMON_TARGETS)

CLEAN_COMMON_TARGETS=$(patsubst %,%.clean,$(COMMON_TARGETS))
CLEAN_TARGETS=$(patsubst %,%.clean,$(TARGETS))

.PHONY: all $(TARGETS) imu2simulink_prereqs clean
all: $(TARGETS)


imu2simulink: imu2simulink_prereqs
$(TARGETS):
	$(MAKE) -C $@

imu2simulink_prereqs: $(imu2simulink_PREREQS)

$(IMU2SIMULINK_PREREQS):
	cp -r $(XSENS_PUBLIC_SOURCE)/$(patsubst src/%,%,$@) imu2simulink

$(CLEAN_COMMON_TARGETS):
	-$(MAKE) -C $(patsubst %.clean,%,$@) clean

clean: $(CLEAN_TARGETS)
	-$(RM) -r $(SRC_CPP_PREREQS)

.PHONY: all clean sensor sensor_clean libs libs_clean
all: sensor libs
clean: sensor_clean libs_clean

sensor:
	$(AT)$(MAKE) -C sensor
sensor_clean:
	$(AT)$(MAKE) -C sensor clean

libs:
	$(AT)$(MAKE) -C libs
libs_clean:
	$(AT)$(MAKE) -C libs clean



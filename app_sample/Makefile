SUBDIRS := face_recognize rpi_detector

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(AT)$(MAKE) -C $@ all

clean:
	$(AT)for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

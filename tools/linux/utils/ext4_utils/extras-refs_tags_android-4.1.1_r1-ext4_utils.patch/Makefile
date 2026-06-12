#
#  Create By Czyong
#
#  make_ext4fs -l 128M -s rootfs.ext4 ./rootfs
#
################################################################################

CC      := gcc
CFLAGS  := -I.
LDFLAGS := -lz
TARGETS := make_ext4fs
DEPS    := $(COBJS:.o=.d)


make_ext4fs_OBJS := make_ext4fs_main.o ext4fixup.o ext4_utils.o output_file.o \
	allocate.o sparse_crc32.o make_ext4fs.o contents.o \
	extent.o indirect.o backed_block.o uuid.o wipe.o \
	sha1.o

COBJS            := $(make_ext4fs_OBJS)
################################################################################

all: $(TARGETS)

make_ext4fs: $(make_ext4fs_OBJS)
	$(CC) $(make_ext4fs_OBJS) -static -o $@ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.d : %.c
	@set -e; $(CC) $(CFLAGS) -MM $< | sed 's,$*.o:,$*.o $*.d:,g' > $@

clean:
	@rm -f $(COBJS)
	@rm -f $(TARGETS)
	@rm -f $(DEPS)

ifneq ("$(MAKECMDGOALS)","clean")
sinclude $(DEPS)
endif

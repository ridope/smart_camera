#BUILD_DIR?=../build/firev_soc/

include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

OBJDIR = ./obj
EXT_LIBDIR = ../../../ext_libs

AMP_LIB_OBJ := $(OBJDIR)/amp_utils.o \
			$(OBJDIR)/amp_comms.o

OBJECTS := $(AMP_LIB_OBJ) \
			$(OBJDIR)/svm_model.o 	\
			$(OBJDIR)/crt0.o 	\
			$(OBJDIR)/main.o

INCLUDES += -I$(EXT_LIBDIR)/amp_lib/include

ifdef WITH_CXX
	OBJECTS += hellocpp.o
endif


all: sram.bin shared.bin


sram.bin: demo.elf
	$(OBJCOPY) -R .shared_ram -O binary $< $@
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

shared.bin: demo.elf
	$(OBJCOPY) -j .shared_ram -O binary $< $@
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

vpath %.a $(PACKAGES:%=../%)

demo.elf: $(OBJECTS)
	$(CC) -Os $(LDFLAGS) -Wl,--print-memory-usage -T linker.ld -N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-L$(EXT_LIBDIR)/amp_lib \
		$(LIBS:lib%=-l%) \
		-lamplib \
		-lgcc

ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)


VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY):$(EXT_LIBDIR)/amp_lib/source

$(OBJDIR)/main.o: main.c $(EXT_LIBDIR)/amp_lib/libamplib.a
	@mkdir -p $(@D)
	$(compile)

$(OBJDIR)/crt0.o: $(CPU_DIRECTORY)/crt0.S
	@mkdir -p $(@D)
	$(assemble)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(compilexx)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(compile)

$(OBJDIR)/%.o: $(EXT_LIBDIR)/amp_lib/source/%.c
	@mkdir -p $(@D)
	$(compile)

$(EXT_LIBDIR)/amp_lib/libamplib.a: $(AMP_LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.S
	@mkdir -p $(@D)
	$(assemble)

clean:
	$(RM) $(OBJECTS) demo.elf *.bin .*~ *~

.PHONY: all clean
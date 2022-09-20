#BUILD_DIR?=../build/firev_soc/

include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

OBJDIR = ./obj
OBJECTS := $(OBJDIR)/amp_send.o \
			$(OBJDIR)/svm_model.o 	\
			$(OBJDIR)/crt0.o 	\
			$(OBJDIR)/amp_utils.o 	\
			$(OBJDIR)/main.o

ifdef WITH_CXX
	OBJECTS += hellocpp.o
endif


all: sram.bin sp.bin


sram.bin: demo.elf
	$(OBJCOPY) -R .private_ram -O binary $< $@
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

sp.bin: demo.elf
	$(OBJCOPY) -j .private_ram -O binary $< $@
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

vpath %.a $(PACKAGES:%=../%)

demo.elf: $(OBJECTS)
	$(CC) -Os $(LDFLAGS) -Wl,--print-memory-usage -T linker.ld -N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		$(LIBS:lib%=-l%) \
		-lgcc

ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)


VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY)

$(OBJDIR)/crt0.o: $(CPU_DIRECTORY)/crt0.S
	@mkdir -p $(@D)
	$(assemble)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(compilexx)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(compile)

$(OBJDIR)/%.o: %.S
	@mkdir -p $(@D)
	$(assemble)

clean:
	$(RM) $(OBJECTS) demo.elf demo.bin .*~ *~

.PHONY: all clean
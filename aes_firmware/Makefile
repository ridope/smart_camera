include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

OBJDIR = ./obj
EXT_LIBDIR = ../ext_libs

TINY_CRYPT_OBJ := $(OBJDIR)/aes_decrypt.o \
				$(OBJDIR)/aes_encrypt.o \
				$(OBJDIR)/ctr_mode.o \
				$(OBJDIR)/ctr_prng.o \
				$(OBJDIR)/ccm_mode.o \
				$(OBJDIR)/cmac_mode.o \
				$(OBJDIR)/utils.o

OBJECTS := $(TINY_CRYPT_OBJ) \
		  $(OBJDIR)/aes.o \
		  $(OBJDIR)/main.o \
		  $(OBJDIR)/crt0.o

INCLUDES += -I$(EXT_LIBDIR)/tinycrypt \
			-I$(EXT_LIBDIR)/tinycrypt/lib/include \
			-I$(EXT_LIBDIR)/tinycrypt/lib/tinycrypt \

ifdef WITH_CXX
	OBJECTS += hellocpp.o
endif

all: sram.bin


sram.bin: demo.elf
	$(OBJCOPY) -O binary $< $@

ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

vpath %.a $(PACKAGES:%=../%)

demo.elf: $(OBJECTS)
	$(CC) -Os $(LDFLAGS) -Wl,-Map=output.map -Wl,--print-memory-usage -T linker.ld -N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-L$(EXT_LIBDIR)/tinycrypt/lib \
		$(LIBS:lib%=-l%) \
		-ltinycrypt \

ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY)

$(OBJDIR)/main.o: main.c $(EXT_LIBDIR)/tinycrypt/lib/libtinycrypt.a
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

$(OBJDIR)/%.o: $(EXT_LIBDIR)/tinycrypt/lib/source/%.c
	@mkdir -p $(@D)
	$(compile)

$(EXT_LIBDIR)/tinycrypt/lib/libtinycrypt.a: $(TINY_CRYPT_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.S
	@mkdir -p $(@D)
	$(assemble)

clean:
	$(RM) $(OBJECTS) *.elf *.bin .*~ *~

.PHONY: all clean
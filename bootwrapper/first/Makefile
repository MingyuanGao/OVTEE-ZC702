# Build an ELF linux image

BOOTLOADER	= boot.S
IMAGE           = ./normal.elf
LD_SCRIPT	= model.lds

AS		= $(CROSS_COMPILE)as -g
LD		= $(CROSS_COMPILE)ld -g

all: $(IMAGE)

clean:
	rm -f $(IMAGE) boot.o

$(IMAGE): boot.o $(LD_SCRIPT) $(KERNEL)
	$(LD) -o $@ --script=$(LD_SCRIPT)

boot.o: $(BOOTLOADER)
	$(AS) -o $@ $<

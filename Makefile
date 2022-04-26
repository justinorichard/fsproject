.PHONY: all
all: boot.iso

.PHONY: run
run: boot.iso
	./run.sh

.PHONY: clean
clean:
	rm -f iso_root boot.iso
	$(MAKE) -C stdlib clean
	$(MAKE) -C kernel clean
	$(MAKE) -C init clean
	$(MAKE) -C hello_world clean
	$(MAKE) -C test_mod clean

.PHONY: stdlib
stdlib:
	$(MAKE) -C stdlib

.PHONY: kernel
kernel: stdlib
	$(MAKE) -C kernel

.PHONY: init
init: stdlib
	$(MAKE) -C init

.PHONY: hello_world
hello_world: stdlib
	$(MAKE) -C hello_world

.PHONY: test_mod
test_mod: stdlib
	$(MAKE) -C test_mod

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
	$(MAKE) -C limine

boot.iso: limine kernel init hello_world test_mod limine.cfg
	rm -rf iso_root
	mkdir -p iso_root
	cp kernel/kernel.elf init/init hello_world/hello_world test_mod/test_mod limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-eltorito-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-eltorito-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label iso_root -o boot.iso
	limine/limine-install boot.iso
	rm -rf iso_root
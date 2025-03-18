	qemu-system-$(ARCH) \
		-M q35 \
		-cdrom $(IMAGE_NAME).iso \
		-boot d \
		$(QEMUFLAGS)

qemu-system-x86_64 -M q35 -drive file=atlas_os-x86_64.iso,if=ide -boot d -m 2G -enable-kvm
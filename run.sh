#!/usr/bin/env bash

qemu-system-x86_64 -drive file=atlas_os-x86_64.iso,format=raw,if=ide -m 4G -debugcon stdio -enable-kvm

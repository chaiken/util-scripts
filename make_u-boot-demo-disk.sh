#!/bin/bash

readonly DTB=/home/alison/gitsrc/u-boot/arch/sandbox/dts/sandbox64.dtb
readonly COMPRESSED_KERNEL=/boot/vmlinuz-"$(uname -r)"
readonly TOOLDIR=/home/alison/gitsrc/linux-trees/linux/scripts
readonly OUTPUT_DIR="$(mktemp -d)"
readonly KERNEL="$OUTPUT_DIR"/uImage
readonly LOADADDR=0x80000000
readonly SHASUM_FILE="$(mktemp)"
readonly DISK_NAME="sdcard.raw"

set -eu

/bin/rm -f "$DISK_NAME"

if [[ ! -f "$DTB" ]]; then
   echo "Compiling devicetree"
   make ARCH=sandbox dtbs
fi

"$TOOLDIR"/extract-vmlinux "$COMPRESSED_KERNEL" > "$OUTPUT_DIR"/vmlinux
mkimage -A sandbox -T kernel -O linux -a "$LOADADDR" -d "$OUTPUT_DIR"/vmlinux "$OUTPUT_DIR"/uImage
shasum "$KERNEL" > "$SHASUM_FILE"

/bin/rm -f "$DISK_NAME"
truncate -s 1000M "$DISK_NAME"
echo -e "label: gpt\n,64M,U\n,128M,L\n,128M,L\n,+,\n" | sfdisk "$DISK_NAME"
sgdisk -v "$DISK_NAME"
readonly lodev="$(sudo /sbin/losetup -P -f --show ${DISK_NAME})"
echo "Loopback device is ${lodev}"

echo " "
echo "Partition 1"
sudo mkfs.vfat -n DTB "$lodev"p1
sudo mount -t vfat "$lodev"p1 /mnt/usb
sudo cp "$DTB" /mnt/usb
ls -l /mnt/usb
sync
sudo umount /mnt/usb

echo " "
echo "Partition 2"
sudo mkfs.ext2 -L kernelA "$lodev"p2
sudo mount -t ext2  "$lodev"p2 /mnt/backup
sudo cp "$KERNEL" /mnt/backup
sync
# shasum checks a hash against a file in $PWD
cd /mnt/backup
shasum  -c "$SHASUM_FILE"
cd -
df -h /mnt/backup
ls -l /mnt/backup
sudo umount /mnt/backup

echo " "
echo "Partition 3"
sudo mkfs.ext2 -L kernelB "$lodev"p3
sudo mount -t ext2  "$lodev"p3 /mnt/usb
sudo cp "$KERNEL" /mnt/usb
sync
# shasum checks a hash against a file in $PWD
cd /mnt/usb
shasum  -c "$SHASUM_FILE"
cd -
df -h /mnt/usb
ls -l /mnt/usb
sudo umount /mnt/usb

/bin/rm "$SHASUM_FILE"
/bin/rm -rf "$KERNEL"

sudo chown "$USER":"$USER" "$DISK_NAME"

#!/usr/bin/env bash

if ! sudo true; then
  echo "Sudo is required to mount and flash"
  exit 1
fi

BLKDEV=$(lsblk -rno LABEL,PATH | grep RP2350 | cut -d" " -f2)
MNTPOINT=$(mktemp -d)

sudo mount $BLKDEV $MNTPOINT
sudo cp build/main.uf2 $MNTPOINT
sync
sudo umount $MNTPOINT

rmdir $MNTPOINT

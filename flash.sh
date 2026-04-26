#!/usr/bin/env bash

BLKDEV=$(lsblk -rno LABEL,PATH | grep RP2350 | cut -d" " -f2)

if [ "$BLKDEV" = "" ]; then
  echo "RP2350 not found"
  exit 1
fi

if ! sudo true; then
  echo "Sudo is required to mount and flash"
  exit 1
fi

echo "Flashing..."

MNTPOINT=$(mktemp -d)

sudo mount $BLKDEV $MNTPOINT
sudo cp build/main.uf2 $MNTPOINT
sync
sudo umount $MNTPOINT

rmdir $MNTPOINT

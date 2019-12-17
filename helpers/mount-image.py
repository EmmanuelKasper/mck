#!/usr/bin/python3
import parted
import sys
import subprocess
import pathlib
from time import sleep
# 512 bytes per sector
SECTOR_SIZE = 512

disk_image = sys.argv[1]
partition_index = int(sys.argv[2])
device = parted.getDevice(disk_image)

try:
  disk = parted.newDisk(device)
except parted._ped.DiskException:
  print(f"{disk_image} doesn't look like a disk image ...")
  sys.exit(1)

offsets = []
for partition in disk.partitions:
  offsets.append({'start': partition.geometry.start * SECTOR_SIZE,
    'size': partition.geometry.length * SECTOR_SIZE})

loop_setup = f"/usr/bin/udisksctl loop-setup --file {disk_image} \
  --offset {offsets[partition_index]['start']} --size {offsets[partition_index]['size']}"
subprocess.run(loop_setup.split(), stdout=subprocess.DEVNULL)

losetup = f"losetup --list --noheadings --output NAME \
  --associated {disk_image}"
loop_device = subprocess.run(losetup.split(), stdout=subprocess.PIPE)\
  .stdout.decode('ascii').rstrip()

print(loop_device)




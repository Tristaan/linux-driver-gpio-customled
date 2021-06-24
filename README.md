# Linux char device driver for gpio interfacing
This is a simple driver which enables writing to a device file which then turns on a simple LED on GPIO #21.

## Building
```
make
```

## Running
Use insmod to enable the module:
```
sudo insmod customled.ko
```

use echo to turn the LED on or off, also check `dmesg` for current LED status or read the first byte of the device file.
```
echo 1 | sudo tee /dev/customled
echo 0 | sudo tee /dev/customled
sudo cat /dev/customled
```

## Details
This driver is built using gcc arm-linux-gnueabihf 8.3.0. The driver is built against the 5.10.17 kernel source.

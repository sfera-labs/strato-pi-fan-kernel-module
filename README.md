# Strato Pi Fan driver kernel module

Raspberry Pi OS (Debian) Kernel module for [Strato Pi Fan](https://www.sferalabs.cc/product/strato-pi-fan/) - the Raspberry Pi B expansion board for temperature monitoring and regulation.

[![Build tests [stable oldstable]](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-apt.yml/badge.svg)](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-apt.yml)
[![Build tests [firmware]](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-fw.yml/badge.svg)](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-fw.yml)

## Compile and Install

*For installation on Ubuntu [read this](https://github.com/sfera-labs/knowledge-base/blob/main/raspberrypi/kernel-modules-ubuntu.md).*

Make sure your system is updated:

    sudo apt update
    sudo apt upgrade

If you are using a Raspberry Pi **4** and a **32-bit** OS, add to `/boot/firmware/config.txt` (`/boot/config.txt` in older versions) the following line: [[why?](https://github.com/raspberrypi/firmware/issues/1795)]

    arm_64bit=0

Reboot:

    sudo reboot

After reboot, install required tools:

    sudo apt install git device-tree-compiler dkms linux-headers-$(uname -r)

Clone this repo:

    git clone --depth 1 https://github.com/sfera-labs/strato-pi-fan-kernel-module.git

    cd strato-pi-fan-kernel-module

### Recommended installation mode: DKMS

This is the recommended mode. It automatically rebuilds and reinstalls the module when new kernel versions are installed.

Register, build and install with DKMS:

    sudo dkms add .
    sudo dkms build -m stratopifan -v $(cat VERSION)
    sudo dkms install -m stratopifan -v $(cat VERSION)

### Advanced installation mode: manual make install (running kernel only)

Use this only if you specifically want to install for the current running kernel version only.

    make clean
    make
    sudo make install

Manual mode does not provide automatic rebuild on kernel upgrades.

### Enable overlay at boot

Add to `/boot/firmware/config.txt` the following line:

    dtoverlay=stratopifan

### Optional non-root access to `/sys/class/stratopifan`

The install process places `99-stratopifan.rules`, which sets owner group `stratopifan` for sysfs entries. To access the sysfs interface without superuser privileges, create the group and add your user, e.g. for user "pi":

    sudo groupadd stratopifan
    sudo usermod -a -G stratopifan pi

Reboot:

    sudo reboot

## Usage

After installation, you'll find the directory `/sys/class/stratopifan/` which gives you access to Strato Pi Fan's functionalities.

To read the temperature measured by the on-board sensor, read the file `/sys/class/stratopifan/sys_temp/temp`.

To set the temperature thresholds for activating and deactivating the fan write respectively to `/sys/class/stratopifan/fan/temp_on` and `/sys/class/stratopifan/fan/temp_off`.

All temperature values are expressed in &deg;C/100, i.e. a value of `4050` corresponds to 40.5&deg;C. Thresholds values range from -128&deg;C to 127.5&deg;C with a 0.5&deg;C resolution.

### Examples

Read temperature and thresholds from shell:

    $ cat /sys/class/stratopifan/sys_temp/temp
    $ cat /sys/class/stratopifan/fan/temp_on
    $ cat /sys/class/stratopifan/fan/temp_off

Write thresholds from shell:

    $ echo 6000 > /sys/class/stratopifan/fan/temp_on
    $ echo 5150 > /sys/class/stratopifan/fan/temp_off
    
Read temperature and set thresholds using Python:

    f = open('/sys/class/stratopifan/sys_temp/temp', 'r')
    val = int(f.read().strip())
    f.close()
    print('Temperature: {} C'.format(val/100.0))
    
    tOn = val + 1000
    tOff = val - 1000
    
    f = open('/sys/class/stratopifan/fan/temp_on', 'w')
    f.write(str(tOn))
    f.close()
    
    f = open('/sys/class/stratopifan/fan/temp_off', 'w')
    f.write(str(tOff))
    f.close()

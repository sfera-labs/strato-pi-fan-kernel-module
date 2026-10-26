# Strato Pi Fan driver kernel module

Debian-based Linux kernel module for [Strato Pi Fan](https://www.sferalabs.cc/product/strato-pi-fan/) - the Raspberry Pi B expansion board for temperature monitoring and regulation.

[![Build tests [stable oldstable]](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-apt.yml/badge.svg)](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-apt.yml)
[![Build tests [firmware]](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-fw.yml/badge.svg)](https://github.com/sfera-labs/strato-pi-fan-kernel-module/actions/workflows/build-fw.yml)

## Installation

For source-based installation, see this [installation guide](https://github.com/sfera-labs/sl-kmod-commons/blob/main/INSTALLATION.md).

If the Sfera Labs APT repository is not configured yet, [add the repository and install its keyring](https://github.com/sfera-labs/apt#install).

Install the package:

    sudo apt update
    sudo apt install stratopifan-dkms

The package registers the module with DKMS, which builds it for kernels with matching headers and rebuilds it when new kernel headers are installed. It also installs the device-tree overlay and udev rules.

Enable the overlay by adding this to `/boot/firmware/config.txt`:

    dtoverlay=stratopifan

On legacy 32-bit Raspberry Pi OS versions that support this option, `arm_64bit=0` may be required. Add it to the same file. [[why?](https://github.com/raspberrypi/firmware/issues/1795)]

The install process creates the `stratopifan` group and places `99-stratopifan.rules`, which sets it as the owner group for sysfs entries. To access the sysfs interface without superuser privileges, add your user to the group, e.g. for user "pi":

    sudo usermod -a -G stratopifan pi

Finally, reboot to apply the configuration:

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

# Strato Pi Fan driver kernel module

Raspberry Pi OS (Debian) Kernel module for [Strato Pi Fan](https://www.sferalabs.cc/product/strato-pi-fan/) - the Raspberry Pi B expansion board for temperature monitoring and regulation. 

## Compile and Install

*For installation on Ubuntu [read this](https://github.com/sfera-labs/knowledge-base/blob/main/raspberrypi/kernel-modules-ubuntu.md).*

Make sure your system is updated:

    sudo apt update
    sudo apt upgrade

If you are using a Raspberry Pi **4** and a **32-bit** OS, add to `/boot/firmware/config.txt` (`/boot/config.txt` in older versions) the following line: [[why?](https://github.com/raspberrypi/firmware/issues/1795)]

    arm_64bit=0
    
Reboot:

    sudo reboot

Clone this repo:

    git clone --depth 1 https://github.com/sfera-labs/strato-pi-fan-kernel-module.git

Make and install:

    cd strato-pi-fan-kernel-module
    make clean
    make
    sudo make install
    
Compile the Device Tree and install it:

    dtc -@ -Hepapr -I dts -O dtb -o stratopifan.dtbo stratopifan.dts
    sudo cp stratopifan.dtbo /boot/overlays/
    
Add to `/boot/firmware/config.txt` (`/boot/config.txt` in older versions) the following line:

    dtoverlay=stratopifan

Optionally, to be able to use the `/sys/class/stratopifan/` files not as super user, create a new group "stratopifan" and set it as the module owner group by adding an udev rule:

    sudo groupadd stratopifan
    sudo cp 99-stratopifan.rules /etc/udev/rules.d/

and add your user to the group, e.g., for user "pi":

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

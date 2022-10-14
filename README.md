# n2l-server
ODROID-N2L JIG-Server (ODROID-C4 app)

### Install Package
build-essential, git, overlayroot, vim, ssh, 

### Base Image
project/n2l/buntu-22.04-4.9-minimal-odroid-c4-hc4-20220705.img
1. apt update && apt upgrade
2. apt install build-essential git overlayroot vim ssh
3. systemctl edit getty@tty1.service (auto login)
    [Service] ExecStart=-/sbin/agetty --noissue --autologin root %I $TERM Type=idle  
    save exit [Ctrl+ k, Ctrl + q]
4. /media/boot/config.ini -> display_autodetect = false, hdmi (800x480p60hz설정),  
   EDID 참조: https://en.wikipedia.org/wiki/Extended_Display_Identification_Data 
5. git clone https://github.com/charles-park/n2l-server
6. project build : make
7. service install : n2l-server/service/install.sh
8. screen off disable
9. emmc resize : ubuntu pc used disk util (4608 MB)
10. image dump : dd if=/dev/sda of=./odroid-n2l-server.img bs=512M count=10
11. test image
12. overlay enable
```
root@odroid:~# update-initramfs -c -k $(uname -r)
update-initramfs: Generating /boot/initrd.img-4.9.277-75
root@odroid:~#
root@odroid:~# mkimage -A arm64 -O linux -T ramdisk -C none -a 0 -e 0 -n uInitrd -d /boot/initrd.img-$(uname -r) /media/boot/uInitrd 
Image Name:   uInitrd
Created:      Wed Feb 23 09:31:01 2022
Image Type:   AArch64 Linux RAMDisk Image (uncompressed)
Data Size:    13210577 Bytes = 12900.95 KiB = 12.60 MiB
Load Address: 00000000
Entry Point:  00000000
root@odroid:~#

overlayroot.conf 파일의 overlayroot=””를 overlayroot=”tmpfs”로 변경합니다.
vi /etc/overlayroot.conf
overlayroot_cfgdisk="disabled"
overlayroot="tmpfs"
```
13. overlay modified/disable  
```
overlayroot.conf 파일의 overlayroot=”tmpfs”를 overlayroot=””로 변경합니다.
vi /etc/overlayroot.conf
overlayroot_cfgdisk="disabled"
overlayroot=""
[get write permission]
odroid@hirsute-server:~$ sudo overlayroot-chroot 
INFO: Chrooting into [/media/root-ro]
root@hirsute-server:/# 

[disable overlayroot]
```
14. final image dump  

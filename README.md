# n2l-server
ODROID-N2L JIG-Server (ODROID-C4 app)

### Install Package
build-essential, git, overlayroot, vim, ssh, minicom

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
8. screen off disable (setterm -blank 0 -powerdown 0 -powersave off 2>/dev/null, echo 0 > /sys/class/graphics/fb0/blank)  
   vi ~/.bashrc (반드시 실행되는 터미널의 bashrc를 수정하여야 함)
   https://wiki.odroid.com/odroid-n2/ubuntu_minimal_quick_guide#disable_screen_blank  
9. emmc resize : ubuntu pc used disk util (4608 MB)
10. image dump : dd if=/dev/sda of=./odroid-n2l-server.img bs=512M count=9
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
[get write permission]
odroid@hirsute-server:~$ sudo overlayroot-chroot 
INFO: Chrooting into [/media/root-ro]
root@hirsute-server:/# 

[disable overlayroot]
overlayroot.conf 파일의 overlayroot=”tmpfs”를 overlayroot=””로 변경합니다.
vi /etc/overlayroot.conf
overlayroot_cfgdisk="disabled"
overlayroot=""

```
14. final image dump  

### NLP(Network Label Printer) 설치 및 등록 방법
1. git clone https://github.com/charles-park/nlp_test
2. apt install nmap
3. build nlp_test
4. n2l-server/app.cfg에 아래의 내용 등록 (app.cfg 내용 참조)  
```
# ----------------------------------------------------------------------------
#
# Network printer 설정
#
# NLP_APP_PATH : 실행파일 위치 및 실행파일명, 없으면 disable
# NLP_IP_ADDR : 프린터 IP, 없으면 자동으로 찾아서 전송(만약 없는 경우에는 프린트 x)
# github.com/charles-park/nlp_test.git을 사용하여 프린트. (nlp_test -h 로 명령검토)
#
# ----------------------------------------------------------------------------
NLP_APP_PATH = /root/nlp_app/nlp_app
# NLP_IP_ADDR = 192.168.20.28
NLP_DISPLAY_R_ITEM = 27
```


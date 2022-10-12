#!/bin/bash
systemctl disable auto_start.service && sync

cp ./auto_start.service /etc/systemd/system/ && sync

systemctl enable auto_start.service && sync

systemctl restart auto_start.service && sync

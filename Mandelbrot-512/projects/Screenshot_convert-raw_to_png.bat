@ECHO OFF
: What software can display raw bitmaps on Linux?
: https://askubuntu.com/questions/147554/what-software-can-display-raw-bitmaps-on-linux#answer-678569
convert.exe -depth 8 -size 1920x1080+0 gray:Screenshot.raw Screenshot.png

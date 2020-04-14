# ping
A CLI based Ping Application written in C++

## Build:

       make

## Execute  

       sudo ./ping {ipv4 | hostname} [-t 0-255]   (Sudo because it uses raw sockets)

       sudo ./ping 0
       sudo ./ping localhost
       sudo ./ping www.google.com
       sudo ./ping www.google.com -t 100

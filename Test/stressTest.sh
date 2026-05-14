#!/bin/bash
for i in {1..1000}
do
   # Open telnet connection, send request, and keep pipe open
   (echo "GET / HTTP/1.1"; echo "Host: localhost"; echo; sleep 10) | telnet 127.0.0.1 8080 &
done

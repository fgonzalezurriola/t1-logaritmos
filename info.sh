#!/bin/bash

echo ">> Sistema operativo:"
cat /etc/os-release | grep PRETTY_NAME

echo
echo ">> Información de CPU:"
lscpu | grep -E 'Model name|CPU\(s\):|Thread|Core|Socket|MHz|L1d cache|L1i cache|L2 cache|L3 cache'

echo
echo ">> Memoria RAM total:"
free -h | grep Mem

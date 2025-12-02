#!/bin/bash
# Simple CPU/RAM check
top -bn1 | grep aimbot
echo "RAM: $(free -h | grep Mem)"

#!/bin/bash
# Grant permissions
termux-toast "Starting Aimbot"
./aimbot &
# Monitor performance
watch -n1 './scripts/performance_test.sh'

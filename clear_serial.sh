#!/bin/bash
# Script to clear serial port before upload

echo "Clearing serial port..."

# Kill any arduino-cli monitor
pkill -f "arduino-cli monitor" 2>/dev/null

# Kill any screen sessions
pkill screen 2>/dev/null

# Kill any minicom
pkill minicom 2>/dev/null

# Force kill anything using the port (macOS specific)
if [ -f /dev/cu.usbserial-02883397 ]; then
    lsof -t /dev/cu.usbserial-02883397 2>/dev/null | xargs -r kill -9 2>/dev/null
fi

echo "Serial port cleared!"
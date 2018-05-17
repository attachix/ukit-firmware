
# U:Kit ESP8266 Firmware - This is the 'smart' firmware for the U:Kit sensor kit
# Copyright (C) 2016, 2018 Slavey Karadzhov <slav@attachix.com>
#
# This file is part of U:Kit ESP8266 Firmware.
#
# U:Kit ESP8266 Firmware is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# U:Kit ESP8266 Firmware is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with U:Kit ESP8266 Firmware.  If not, see <http://www.gnu.org/licenses/>.

# This program simulates MCU running with TSB in it.
import serial
import struct
import sys
from time import sleep

TSB_REQUEST = "?"
TSB_CONFIRM = "!"
TSB_CMD_WRITE_FLASH = "F"
TSB_CMD_READ_USER_DATA = "c"

def usage():
    print "Usage: %s [<com-port> [<com-speed>]]" % sys.argv[0]


def read(stream, size=1024, timeout=None):
    """Read size of data. Finish if no data come during timeout.
    Timeout is automatically prolonged when receive data.
    See: https://github.com/mvyskoc/avrtsb/blob/master/avrtsb/tsbloader.py
    """
    if timeout == None:
        timeout = stream.timeout * 1000.

    old_timeout = stream.timeout
    stream.timeout = timeout / 1000.
    data = []
    try:
        num_bytes = 0  # Number o received bytes
        while num_bytes < size:
            new_data = stream.read(size - num_bytes)
            num_bytes += len(new_data)
            data.append(new_data)

            # Check timeout
            if new_data == '':
                break
    finally:
        stream.timeout = old_timeout

    return ''.join(data)


if __name__ == "__main__":
    timeout = 0.700  # seconds
    comPort = '/dev/ttyUSB0'
    baudRate = 9600

    commandLength = len(sys.argv)
    if  commandLength >= 2:
        comPort = sys.argv[1]
    if len(sys.argv) == 3:
        baudRate = sys.argv[2]

    pageSize = 64 # in "words" = 2 * bytes
    deviceInfo = struct.pack("3sBBBBBBBBBBBBB1s",
            "tsb",  # 3 bytes ASCII identifier "TSB"
            0x1a, 0x1f,  # 2 bytes firmware date identifier
            0xf0,  # 1 byte firmware status identifier (reserved)
            0x1e, 0x94, 0x12,  #  3 signature bytes in the format of ATMEL signature bytes
            pageSize,  # 1 byte the Pagesize in Words (with ATtinys this may be 32, 64 or 128 Byte)
            0xd0, 0x1e,  # 2 bytes of available Flash memory (total free memory in bytes for Application minus Bootloader alotted memory)
            0xff, 0x00,  # 2 bytes EEPROM size)
            0x0c, 0x94,
            TSB_CONFIRM
            )

    userData = chr(0x01)
    userData = userData.rjust(32, chr(0xFF)) + TSB_CONFIRM

    for b in deviceInfo:
        print "%c -> %x" % (b,ord(b))

    stream = serial.Serial(
        port=comPort,
        baudrate=baudRate,
        timeout=timeout
    )

    if not stream.isOpen():
        print "[MCU] ?! We are not connected ?? "
        sys.exit(2)

    print "[MCU] Connected..."

    ready = False
    offset = 0
    currentCommand = None
    size = 1024
    buf = ""
    while True:
        chunk = read(stream, size, timeout)
        if not len(chunk):
            # print "\n[MCU] Communication was lost or timed out."
            continue

        if not ready:
            buf = buf + chunk

            offset = buf.find("@@@") # check if the buffer contains @@@
            if offset != -1:
                password = ""
                try:
                    password = buf[offset+4:]
                except IndexError as e:
                    pass
                print "\n[MCU] Got password: [%s]" % password
                ready = True
                stream.write(deviceInfo)
                stream.flush()
                size = 1
            continue

        buf = chunk
        if currentCommand is None:
            command = buf[0]  # the first symbol is the command
            currentCommand = command
            if command == TSB_CMD_WRITE_FLASH:
#                 sleep(0.580)
                stream.write(TSB_REQUEST)
                stream.flush()
            elif command == TSB_CMD_READ_USER_DATA:
                stream.write(userData)
                stream.write(TSB_CONFIRM)
                stream.flush()
            continue

        print "Current Command: %s, Buffer[0]: %s" % (currentCommand, buf[0])

        if currentCommand == TSB_CMD_WRITE_FLASH:
            if buf[0] == TSB_CONFIRM:
                buf = read(stream, 2*pageSize+1, 3)
                print "\n[MCU]: Got packet %s" % ":".join("{:02x}".format(ord(c)) for c in buf)
#                 sleep(0.780)
                stream.write(TSB_REQUEST)
                stream.flush()
            elif buf[0] == TSB_REQUEST:
                print "\n[MCU]: Got all data"
                stream.write(TSB_CONFIRM)
                stream.flush()
                size = 1
                currentCommand = None
            else:
                print "\n[MCU]: Unknown start: %s" % buf[0]

        else:
            print "\n[MCU]: Unsupported command: %s" % buf[0]

    stream.close()

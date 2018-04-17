
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

import argparse
import os
import sys

if __name__ == '__main__':
    if len(sys.argv) == 2:
        source = sys.argv[1]
        fp = open(source, 'r')
    else:
        fp = sys.stdin
    line = fp.readline()
    while line:
        bb = line.split(',')
        for b in bb:
            sys.stdout.write(chr(int(b)))
        line = fp.readline()

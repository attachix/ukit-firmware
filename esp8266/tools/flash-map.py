
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

sector = 0x1000

map = {
	0x0 : 'Boot Loader',
	0x1000: 'Boot Config',
	0x2000:  'ROM 0',
	0x100000: 'Spiffs 0',
	(0x400000 / 2) + (sector * 2) : 'Rom 1' # 
}

for i in range(0,4*256):
	
	address = i * sector
	size = ((i + 1) * sector) / 1024
	print "Sector: %d, Address: 0x%x, K: %d" % (i, address , size)
	if address in map:
		print "^-- Start of %s" % map[address]
	if size % 1024 == 0:
		print "^===== End of %d MB" % (size / 1024) 

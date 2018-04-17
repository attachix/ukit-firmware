
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

# This is a python file that tests the communication with MCU2
import time
import serial
import sys

PROTOCOL_READY = chr(0x5a)

commands = {
    # command-name -> 0 - timeout in seconds, 1 - code,  2 - first param, 3- second param, etc.
    "change-mode": [3*60, 0x30, ],
    "smoke":       [70, 0x31, 1, 2], # Level, Count
    "temperature": [70, 0x32, 38], # Degrees
    "motion":      [70, 0x33, 99], 
    "battery":     [70, 0x34, 200], #   
}


def usage():
    print "Usage: %s <command> [<com-port> [<com-speed>]]" % sys.argv[0]
    print "\tAvailable commands: %s\n" % commands.keys()
    
    
def sendCommand(stream, command):
    pass
    

if __name__ == "__main__":
    
    comPort = '/dev/ttyUSB0';
    baudRate = 115200;
    
    commandLength = len(sys.argv); 
    if(commandLength == 1 or commandLength > 4 or \
       sys.argv[1] not in commands 
    ):
        usage();
        sys.exit(1);
        
    currentCommand = sys.argv[1]
    commandBits = commands[currentCommand]
    if  commandLength >= 3:
        comPort = sys.argv[2];
    if len(sys.argv) == 4:
        baudRate = sys.argv[3];
        
    stream = serial.Serial(
        port=comPort,
        baudrate=baudRate,
        timeout = commandBits[0]
    )
    
    if not stream.isOpen():
        print "[Client] ?! We are not connected ?? "
        sys.exit(2)
    
    print "[Client] Connected..."
    
    ready = False
    start = 0
    lastChar = ''
    while True:
        buf = stream.read(1)
        if not len(buf):
            print "\n[Client] Communication was lost or timed out." 
            break
        
        if buf == chr(0x0) and lastChar=='l':
            start = 0
            ready = False
            lastChar = ''
            print "[Client] The device was reset"
            continue
        
        sys.stdout.write(buf)
        lastChar = buf
        
        if not ready:
            if buf == PROTOCOL_READY:
                print "[Client] MCU is ready"
                ready = True
                stream.write(chr(commandBits[1]))
                if len(commandBits) >= 3 and \
                commandBits[2] is not None:
                    stream.write(chr(commandBits[2]))
                if len(commandBits) >= 4 and \
                commandBits[3] is not None:
                    stream.write(chr(commandBits[3]))
                stream.flush()
                time.sleep(0.5) # waiting for the serial port time to receive the data
                print "[Client] Waiting for response..."
                start = time.time();
        elif ord(buf) in [0, 1, PROTOCOL_READY]:
            print "[Client] Got response: 0x%x | %s (sec: %d)\n" % (ord(buf), buf, time.time() - start)
        
    stream.close()
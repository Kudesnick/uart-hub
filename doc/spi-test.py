#!/usr/bin/env python

import time
import sys
import spidev

spi = spidev.SpiDev()
spi.open(0,0)

spi.bits_per_word = 8
spi.lsbfirst = False
spi.max_speed_hz = 16000000 # 63 MHz - MOSI limit, 16 MHz - MISO limit
spi.mode = 0b00 # CPOL0|CPHA0

spi.cshigh = True
spi.cshigh = False
val = spi.xfer3([0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38])
spi.cshigh = True
print "Result: ", str(val)

spi.close()
sys.exit(0)

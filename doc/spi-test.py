#!/usr/bin/env python

import time
import sys
import spidev

spi = spidev.SpiDev()
spi.open(0, 0)

def send_frame(ch, data):
    global spi

    send_data = [data & 0xff, ch & 0xFF, (ch >> 8) & 0xff, (ch >> 16) & 0xff]

    spi.cshigh = True
    spi.cshigh = False
    ret = spi.xfer3(send_data)
    spi.cshigh = True

    return ret

spi.bits_per_word = 8 # other value not supported for BCM2835
spi.lsbfirst = False
spi.max_speed_hz = 16000000 # 63 MHz - MOSI limit, 16 MHz - MISO limit
spi.mode = 0b00 # CPOL0|CPHA0

for i in range(0, 10):
    val = send_frame(ch[i], 0x10 + i)
    print "Result: ", str(val)

spi.close()
sys.exit(0)

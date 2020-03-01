#!/usr/bin/env python

import time
import sys
import spidev

spi = spidev.SpiDev()
spi.open(0, 0)

ch = [1 << i for i in range(0, 10)]

def send_frame(ch, data):
    global spi

    send_data = [data & 0xff, ch & 0xFF, (ch >> 8) & 0xff, (ch >> 16) & 0xff]

    spi.cshigh = True
    spi.cshigh = False
    ret = spi.xfer3(send_data)
    spi.cshigh = True

    return ret

def send(ch, data):
    if isinstance(data, int): data = [data]
    if not isinstance(data, list): return

    res = []
    for d in data: res.extend(send_frame(ch, d))

    return res

spi.bits_per_word = 8 # other value not supported for BCM2835
spi.lsbfirst = False
spi.max_speed_hz = 16000000 # 63 MHz - MOSI limit, 16 MHz - MISO limit
spi.mode = 0b00 # CPOL0|CPHA0

for i in range(0, 10):
    data = [(j << 4) + i for j in range(0, 16)]
    val = send(ch[i], data)
    print "Result: ", str(val)

spi.close()
sys.exit(0)

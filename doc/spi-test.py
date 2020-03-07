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
    return spi.xfer(send_data)

def ret_parse(data):
    global ch

    if not isinstance(data, list):
        return

    if len(data) != 4:
        return None

    mask = data[1] | (data[2] << 8) | (data[3] << 16)

    if ch.count(mask) < 1:
        return None

    channel = ch.index(mask)

    buf[channel].append(data[0])

    if (1 <= len(buf[channel]) <= 2):
        if data[0] != 0x55:
            print "Parse preamble error!"
        elif len(buf[channel]) >= 6 and len(buf[channel]) == (buf[channel][3] + 3):
            #check sum
            chsm = 0
            for i in range(2, buf[channel][3] + 2):
                chsm = chsm + buf[channel][i]

            if (chsm != buf[channel][-1]):
                print "Parse checksum error!"
            else:
                print "ch", str(channel), ":", str(buf[channel])

            buf[channel].clear()

    return

def send(ch, data):
    if isinstance(data, int): data = [data]
    if not isinstance(data, list): return

    res = []
    for d in data: ret_parse(send_frame(ch, d))

    return

spi.bits_per_word = 8 # other value not supported for BCM2835
spi.lsbfirst = False
#  cdiv    speed
#     2    125.0 MHz
#     4     62.5 MHz
#     8     31.2 MHz
#    16     15.6 MHz
#    32      7.8 MHz
#    64      3.9 MHz
#   128     1953 kHz
#   256      976 kHz
#   512      488 kHz
#  1024      244 kHz
#  2048      122 kHz
#  4096       61 kHz
#  8192     30.5 kHz
# 16384     15.2 kHz
# 32768     7629 Hz
spi.max_speed_hz = 16000000 # 63 MHz - MOSI limit, 16 MHz - MISO limit
spi.mode = 0b00 # CPOL0|CPHA0
spi.cshigh = False

for i in range(0, 10):
    data = [0x55, 0x55, 0xFE, 0x03, 0x0E, 0x0E]
    val = send(ch[i], data)
    print "Result: ", str(val)

spi.close()
sys.exit(0)

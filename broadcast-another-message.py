#!/usr/bin/python3

from socket import *

def sendeth(dst, src, eth_type, payload, interface = "enp30s0"):
  assert(len(src) == len(dst) == 6) # 48-bit ethernet addresses
  assert(len(eth_type) == 2) # 16-bit ethernet type
  s = socket(AF_PACKET, SOCK_RAW)
  s.bind((interface, 0))
  return s.send(dst + src + eth_type + payload)

if __name__ == "__main__":
  print("Sent %d-byte Ethernet packet on enp30s0" %
  sendeth(b"\xFF\xFF\xFF\xFF\xFF\xFF",
          b"\x30\x9C\x23\x85\xCB\xB3",
          b"\x7B\x00",
          b"Brand new message for everyone! This frame shall be larger than 64 bytes - Even more!+\x00"))
        # I must end my string with a null character!

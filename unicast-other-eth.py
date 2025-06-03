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
  sendeth(b"\x00\x00\x12\x00\x00\x02",
          b"\x30\x9C\x23\x85\xCB\xB3",
          b"\x7A\x05",
          b"Nobody should get this!"))

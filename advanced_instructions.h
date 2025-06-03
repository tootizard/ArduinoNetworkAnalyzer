// High-level instructions that utilize those in instructions.h
// Meant to ease the development of higher level programs.

// ===== Advanced Instructions =============================
//Read Register - Read any MAC, MII, or ETH register, changing bank automatically.
uint8_t read_register(uint8_t reg) {
	// Clear and set the bank, even if we're in the right bank.
	bfc(ECON1 & ADDRESS_MASK, ECON1_BSEL1 | ECON1_BSEL0);
	bfs(ECON1 & ADDRESS_MASK, (reg & BANK_MASK) >> 5);	// Bank is stored in bits 6 and 7 so shift right 5.

	// Read the register.
	return rcr(reg & ADDRESS_MASK, reg & MACMII_FLAG_MASK);
}

//Write Register - Write to any MAC, MII, or ETH register, changing bank automatically.
void write_register(uint8_t reg, uint8_t data) {
	// Clear and set the bank, even if we're in the right bank.
	bfc(ECON1 & ADDRESS_MASK, ECON1_BSEL1 | ECON1_BSEL0);
	bfs(ECON1 & ADDRESS_MASK, (reg & BANK_MASK) >> 5);	// Bank is stored in bits 6 and 7 so shift right 5.
	// Write to the register.
  wcr(reg & ADDRESS_MASK, data);
}

// Several buffer-related registers come as pairs to control the Low and High byte.
// The high byte register is always 1 address later than the low byte.
// Pass the address of the low-byte register this command to read both registers into a 2 byte result.
uint16_t read_register_pair(uint8_t low_reg) {
  return read_register(low_reg) | (read_register(low_reg+1) << 8);
}

// Several buffer-related registers come as pairs to control the Low and High byte.
// The high byte register is always 1 address later than the low byte.
// Pass the address of the low-byte register and 2 bytes of data to this command to write to both registers.
void write_register_pair(uint8_t low_reg, uint16_t data) {
  write_register(low_reg, data);
  write_register(low_reg+1, data >> 8);
}

// ===== PHYS registers ===============================
// Read PHYS register - Use the special MAC registers (MII) to interact with PHYS.
// See section 3.3.1 on page 19 of the ENC28J60 datasheet for details.
uint16_t read_phy (uint8_t address) {
	write_register(MIREGADR, address);
	write_register(MICMD, MICMD_MIIRD);
	while (read_register(MISTAT) & MISTAT_BUSY)
		;
	write_register(MICMD, 0x00);
	return read_register_pair(MIRDL);
}

// Write PHYS register - Use the special MAC registers (MII) to interact with PHYS.
// See section 3.3.2 on page 19 of the ENC28J60 datasheet for details.
void write_phy (uint8_t address, uint16_t data) {
	write_register(MIREGADR, address);
	write_register_pair(MIWRL, data);
	while (read_register(MISTAT) & MISTAT_BUSY)
		;
}

// Scan PHY register - Probably won't implement.
// See section 3.3.3 on page 19 of the ENC28J60 datasheet for details.
// scan_phy

// ===== More Advanced Instructions ========================

// Setup the ENC28J60 - Call after a src() to prepare the ENC for sending and receiving data.
// See Section 6.0 on page 33 of the ENC28J60 datasheet for details.
void initialize_enc() {
  // RX Buffer - Section 6.1, page 35.
  write_register_pair(ERXSTL, RX_START_ADDRESS);
  write_register_pair(ERXNDL, RX_END_ADDRESS);
  write_register_pair(ERXRDPTL, RX_START_ADDRESS);

  // TX Buffer - Section 6.2, page 35.
  // No explicit action is required to initialize the transmit buffer.
  // Everything that is not used by the receive buffer is considered
  // the transmit buffer.

  // RX Filter.
  // Specific filters can be set, see Section 8.0 on page 48.
  //write_register(ERXFCON, 0x00); // Clearing this register allows all packets to be accepted. 
  //write_register(ERXFCON, 0b10000001); // Unicast or Broadcast only.
  write_register(ERXFCON, 0b00000010); // Mulitcast only.

  // Wait for Oscillator Start-up Timer (OST) before modifying MAC and PHY registers.
  // Notice that the output of read_register is bitwise NOT'd (~).
  while(~read_register(ESTAT) & ESTAT_CLKRDY)
    ;

  // Initialize MAC Settings - Section 6.5, page 36.
  // Enable the MAC to receive packets.
  write_register(MACON1, MACON1_MARXEN);  // Half Duplex
  //write_register(MACON1, MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);  // Full Duplex

  // Setup TX settings for automatic padding and CRC generation as well as preferred duplex.
  write_register(MACON3, MACON3_PADCFG2 | MACON3_PADCFG1 | MACON3_PADCFG0 | MACON3_TXCRCEN); // Half Duplex
  //write_register(MACON3, MACON3_PADCFG2 | MACON3_PADCFG1 | MACON3_PADCFG0 | MACON3_TXCRCEN| MACON3_FULDPX); // Half Duplex

  // Setup the "Defer transmission on collision" bit.
  write_register(MACON4, MACON4_DEFER);

  // Set the maximum frame length that we can RX or TX.
  write_register_pair(MAMXFLL, MAXIMUM_FRAME_LENGTH);

  // Setup the back to back interpacket gap register.
  write_register(MABBIPG, 0x12);  // Half Duplex
  //write_register(MABBIPG, 0x15);  // Full Duplex

  // Setup the non back to back interpacket gap register.
  write_register_pair(MAIPGL, 0x0C12);  // Half Duplex
  //write_register(MAIPGL, 0x12);  // Full Duplex

  // I did not touch the Retransmission and Collision window registers MACLCON1 and MACLCON2 defaults.

  // Set the local MAC address.
  // MAC addresses are 48 bits long.
  // MAADDR1 contains the highest bits while MAADDR6 contains the lowest bits.
  // We must shift the MAC address values into each appropriate register.
  write_register(MAADR1, MAC_ADDRESS >> 40);
  write_register(MAADR2, MAC_ADDRESS >> 32);
  write_register(MAADR3, MAC_ADDRESS >> 24);
  write_register(MAADR4, MAC_ADDRESS >> 16);
  write_register(MAADR5, MAC_ADDRESS >> 8);
  write_register(MAADR6, MAC_ADDRESS);

  // Initialize PHY Settings
  write_phy(PHCON1, 0x0000); // Half Duplex
  //write_phy(PHCON1, 0x0100); // Full Duplex

  write_phy(PHCON2, 0x0100); // Half Duplex
  // For full-duplex, no other option is needed.

  // ===== My custom garbage ========================================================

  // Reset the TX buffer start. - No idea if still necessary.
  write_register_pair(EWRPTL, 0x0000);
  write_register_pair(ETXSTL, 0x0000);

  // Enable interrupts on packet reception.
  // Used in receive_packet().
  write_register(EIE, EIE_PKTIE);
}

// Transmit a packet - See Section 7.1 on page 39 of the ENC28J60 datasheet for more details.
// Destination is an array containing the destination MAC address, message is an unsigned character array containing the message.
void send_packet(uint8_t destination[], uint8_t message[]){
  // Write the per-packet control byte.
  // I don't want to override the MACON3 register per packet at this point, so set it all to 0.
  uint8_t control_byte[] = {TX_PACKET_OVERRIDE_BYTE};
  wbm(control_byte, CONTROL_BYTE_SIZE);

  // Write the destination MAC address to TX buffer.
  wbm(destination, MAC_ADDRESS_SIZE);

  // Write the source MAC address (Our MAC) to TX buffer.
  wbm(MAC_ADDRESS_ARRAY, MAC_ADDRESS_SIZE);

  // Write the size of the ethernet frame to TX buffer.
  wbm(FRAME_LENGTH, FRAME_LENGTH_SIZE);

  // Write the data payload (message) into TX buffer.
  wbm(message, MESSAGE_LENGTH);

  

}

// Receive a packet - See Section 7.2 on page 45 of the ENC28J60 datasheet for more details.
// Pass in the buffer to hold the packet
// Return the size/lenth of frame.
uint16_t receive_packet(uint8_t buffer[], uint16_t& next_packet_pointer, bool debug=false) {

  // I could check if EIR_PKTIF is still set and skip the rest of the logic until it has been cleared be setting PKTDEC.
  // Enable packet reception.
  bfs(ECON1 & ADDRESS_MASK, ECON1_RXEN);

  // Wait for a packet...
  // Note the "!" Logical NOT.
  while(!(read_register(EIR) & EIR_PKTIF))
    ;

  // Stop receiving packets for the time being.
  bfc(ECON1 & ADDRESS_MASK, ECON1_RXEN);

  // Read the next packet pointer and status vector.
  rbm(buffer, 6);

  next_packet_pointer = (buffer[1] << 8) | buffer[0];
  uint16_t frame_size = (buffer[3] << 8) | buffer[2];

  if (debug) {
    // Print status vector.
    Serial.println("STATUS VECTOR:");
    Serial.println(buffer[4], BIN);
    Serial.println(buffer[5], BIN);
  }

  // Actually read in the Ethernet frame now.
  rbm(buffer, frame_size);

  // Update ERXRDPT to the value of next_packet_pointer - Section 7.2.4 Page 47 - FREEING RECEIVE BUFFER SPACE.
  // This is extremely important! If you forget it you can't receive more than a few packets.
  write_register_pair(ERXRDPTL, next_packet_pointer);

  // Update the read pointer to the next packet's location.
  // next_packet_pointer is never set to an odd address.
  // In other words, if an odd-lengthed packet is received,
  // ERDPT will be one byte off of the packet's location.
  // I found no reference to this behavior in the datasheet or errata.
  write_register_pair(ERDPTL, next_packet_pointer);

  return frame_size;
}
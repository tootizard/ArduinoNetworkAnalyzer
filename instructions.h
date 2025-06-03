// ENC28j60 SPI Instructions

// Operation Codes:
#define RCR_OP 0x00
#define RBM_OP 0x3a
#define WCR_OP 0x40
#define WBM_OP 0x7a
#define BFS_OP 0x80
#define BFC_OP 0xa0
#define SRC_OP 0xFF

// Miscellaneous
#define DUMMY_BYTE 0x00

// ===== Helper Functions =================================
// Set SPI chip select low (start) or high (end).
void enableChip() { digitalWrite(chipSelectPin, LOW); }
void disableChip() { digitalWrite(chipSelectPin, HIGH); }

// ===== Commands =========================================
// Read Control Register - Cannot read from PHY directly.
uint8_t rcr(uint8_t address, bool macmiiflag) {
  uint8_t result = NULL;
  enableChip();
  SPI.transfer(RCR_OP | address);
  if (macmiiflag == true) { SPI.transfer(DUMMY_BYTE); }
  result = SPI.transfer(DUMMY_BYTE);
  disableChip();
  return result;
}

// Read Buffer Memory.
void rbm(uint8_t buffer[], uint16_t size) {
  enableChip();
  SPI.transfer(RBM_OP);
  for (uint16_t i=0; i<size; i++)
    buffer[i] = SPI.transfer(DUMMY_BYTE);
  disableChip();
}

// Write Control Register - Cannot write to PHY directly.
void wcr(uint8_t address, uint8_t data) {
  enableChip();
  SPI.transfer(WCR_OP | address);
  SPI.transfer(data);
  disableChip();
  return;
}

// Write Buffer Memory.
void wbm(uint8_t buffer[], uint16_t size) {
  enableChip();
  SPI.transfer(WBM_OP);
  for (uint16_t i=0; i<size; i++)
    SPI.transfer(buffer[i]);
  disableChip();
}

// Bit Field Set - Only usable on ETH registers. Logically OR data with the register.
void bfs(uint8_t address, uint8_t data) {
  enableChip();
  SPI.transfer(BFS_OP | address);
  SPI.transfer(data);
  disableChip();
}

// Bit Field Clear - Only usable on ETH registers. Logically NOT data, then AND with the register.
void bfc(uint8_t address, uint8_t data) {
  enableChip();
  SPI.transfer(BFC_OP | address);
  SPI.transfer(data);
  disableChip();
}

// System Reset Command.
void src() {
  enableChip();
  SPI.transfer(SRC_OP);
  disableChip();
  // Add a little delay to gaurantee the chip reset.
  delay(100);
}

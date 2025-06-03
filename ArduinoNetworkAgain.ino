#include <SPI.h>
#include "variables.h"
#include "registers.h"
#include "instructions.h"
#include "advanced_instructions.h"

// New display stuff starts here 11/23/24
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//uint8_t buffer[MAXIMUM_FRAME_LENGTH];
uint8_t buffer[MY_BUFFER_SIZE];
//uint8_t buffer[100];
static uint16_t next_packet_pointer;
uint16_t frame_size;

// Receive, Print, and Quit
// Working packet reception, returns false unless the special "quit" frame is received.
// Meant to be used in while loop, breaking on return of true.
// More for debugging than anything else.
bool rec_print_quit(uint8_t buffer[], uint16_t next_packet_pointer, uint16_t frame_size){
  Serial.println("Waiting for packet...");
  frame_size = receive_packet(buffer, next_packet_pointer, true); // <- Next packet pointer is not passed back out of the function.

  // Print test packet:
  Serial.print("Next packet location: "); Serial.println(next_packet_pointer);
  Serial.print("Rx read pointer: "); Serial.println(read_register_pair(ERXRDPTL));
  Serial.print("Rx write pointer: "); Serial.println(read_register_pair(ERXWRPTL));
  Serial.print("Packet size: "); Serial.println(frame_size);
  Serial.print("Full Packet: ");
  for (uint16_t i=0; i<frame_size; i++){
    if (i % 16 == 0) {
      Serial.println("");
    }
    if (buffer[i] < 16) {Serial.print("0");} Serial.print(buffer[i], HEX); Serial.print(" ");
  }
  
  Serial.println("");

  // Print the actual message as text.
  Serial.println("Packet Payload:");
  for (uint16_t i=FRAME_HEADER_OFFSET; i<frame_size; i++){
    // Stop at first null character.
    if(buffer[i] == 0x00)
      break;
    Serial.print((char)buffer[i]);
  }

  Serial.println("");

  // Decrement the packet counter.
  bfs(ECON2 & ADDRESS_MASK, ECON2_PKTDEC);
  Serial.print("Packet count: "); Serial.println(read_register(EPKTCNT));

  // Check for quit string, The letter Q (0x51) and a NULL (0x00) after FRAME_HEADER_OFFSET.
  if(buffer[FRAME_HEADER_OFFSET] == 0x51 && buffer[FRAME_HEADER_OFFSET + 1] == 0x00) {
    Serial.println("Quit packet received. Ending program loop...");
    return true;
  }
  return false;
}

void decodeCDP(uint8_t buffer[], uint16_t frame_size, bool debug=false){
  Serial.println("CDP packet discovered!");
  // The first TLV is so many bytes into the frame.
  // We don't want to read the Cyclic Redundancy Check (CRC) because that
  // would cause us to read the buffer outside of our frame.
  for (uint16_t i = FIRST_CDP_TLV_OFFSET; i < frame_size - CRC_BYTES; i++){
    // Type and length are both 2 bytes.
    uint16_t type = (buffer[i] << 8 | buffer[i+1]);
    uint16_t length = (buffer[i+2] << 8 | buffer[i+3]);

    if(debug){
      Serial.print("Type: ");
      Serial.print(type, HEX);
      Serial.print(", Value: ");
    }

    // Reset the display 
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.cp437(true);

    // Not done yet - Left off 8/3/24 - Frame size is low for the test CDP packet!
    // Have to declare outside of my switch case.
    uint16_t result_placeholder = 0;
    uint16_t offset = CDP_TLV_VALUE_OFFSET;
    uint32_t number_of_addresses = 0;
    uint8_t protocol_field_type = 0;
    uint8_t protocol_field_length = 0;
    uint8_t address_length = 0;
    switch(type) {
      case CDP_DEVICE_ID:
      case CDP_PORT_ID:
      case CDP_SW_VERSION:
      case CDP_PLATFORM:
      case CDP_VTP_MANAGEMENT_DOMAIN:
        // These are all ascii, so print each character.
        // Length includes the bytes used in Type (2) and Length (2).
        // In other words, the Value begins 4 bytes into length.
        for (uint16_t j = CDP_TLV_VALUE_OFFSET; j < length; j++){
          Serial.print((char)buffer[i+j]);
          display.print((char)buffer[i+j]);
        }
        break;
      case CDP_NATIVE_VLAN_ID: // Left off here 11/2/24 - Arduino doesn't like variables declared within case...
        for (uint8_t tracker=0; tracker<length-CDP_TLV_VALUE_OFFSET; tracker++ ){
          result_placeholder = (result_placeholder << 8) | buffer[i+CDP_TLV_VALUE_OFFSET+tracker];
        }
        Serial.print(result_placeholder);
        display.print(result_placeholder);
        break;
      case CDP_ADDRESS:
      case CDP_MANAGEMENT_ADDRESS:
        // These have a complex format.
        // First four bytes after Length are the "number of addresses" the device decided to give us.
        // Number of addresses is a four-byte field.
        number_of_addresses = (buffer[i+offset] << 24) | (buffer[i+offset+1] << 16) | (buffer[i+offset+2] << 8) | buffer[i+offset+3];
        offset += 4;
        // For each address, there are a few fields to parse.
        for (uint32_t current_address = 0; current_address < number_of_addresses; current_address++){
          // 1-byte protocol field type and field length.
          protocol_field_type = buffer[i+offset];
          offset += 1;
          protocol_field_length = buffer[i+offset];
          offset += 1;
          // Make sure we have enough length in this TLV for the address.
          // Note that "2" is the length of the "address length" field soon to come.
          if(length < protocol_field_length + 0x02){
            Serial.print("Error: Not enough space for protocol + address length");
          }
          // Skip the protocol specifier. <- Obtained from tcpdump source code.
          // I imagine IP (0xCC) is the only one.
          offset += protocol_field_length;
          address_length = (buffer[i+offset] << 8) | buffer[i+offset+1];
          offset += 2;
          // This one is pretty bad...
          // If the protocol type is NLPID,
          // and the length is 1 bit,
          // and the protocol specifier is the NLPID code for an IP address (0xCC),
          // and the address length is 4.
          // Then the IP address can be read as-is in a four-byte (1-per-octet) format.
          // NOTE: We wait to read the protocol specifier (buffer[i+offset-3]) until
          // this moment because variable-length, byte-by-byte readings are a major pain.
          if (protocol_field_type == PT_NLPID && protocol_field_length == 1 && buffer[i+offset-3] == NLPID_IP && address_length == 4){
            // Print each octet.
            Serial.print(buffer[i+offset]); Serial.print(".");
            Serial.print(buffer[i+offset+1]); Serial.print(".");
            Serial.print(buffer[i+offset+2]); Serial.print(".");
            Serial.println(buffer[i+offset+3]);
            display.print(buffer[i+offset]); display.print(".");
            display.print(buffer[i+offset+1]); display.print(".");
            display.print(buffer[i+offset+2]); display.print(".");
            display.print(buffer[i+offset+3]);
          }
          // Check for PT_IEEE_802_2 - IPv6 stuff. I'll not add in this version.
          else if (false){
            Serial.print("IPv6 placeholder...");
          }
          // Print raw or toss anything else that comes in
          else {
            Serial.print("Generic case placeholder.");
          }
          offset += address_length;
        }
        break;

      case 0x0000:
        // There is no CDP 0x0000 type.
        // If this happens we probably hit padding, so end it.
        // Setting i = frame_size should stop the for loop.
        i = frame_size;
        break;
      default:
        Serial.print("Not implemented: type ");
        Serial.print(type);
        display.print("Not implemented: type ");
        display.print(type);
        break;
    }

    // End the line.
    Serial.println("");

    // Show on the display!
    display.display();
    // Wait until the button is pressed to move on.
    // Await a distinct button press by using two loops.
    while (digitalRead(CYCLE_BUTTON) == HIGH) {
      delay(50);
    }
    while (digitalRead(CYCLE_BUTTON) == LOW) {
      delay(50);
    }

    // Update i, we don't want to re-read part of the last TLV.
    // The subtracted 1 is used to offset the i++ in the for loop's condition.
    i = i + length - 1;
    if (debug == true) {
      Serial.print("i: ");
      Serial.print(i);
      Serial.print(" framesize - CRC_BYTES: ");
      Serial.println(frame_size-CRC_BYTES);
    }
  }
}

void decodeLLDP(uint8_t buffer[], uint16_t frame_size){
  // Reset the display 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
  // Tell them how it is.
  Serial.println("LLDP not yet implemented");
  display.println("LLDP not yet implemented");
}

void setup() {
  // Prep the cycle button.
  pinMode(CYCLE_BUTTON, INPUT);

  // Prep Arduino Communication.
  Serial.begin(9600);
  SPI.begin();

  // Start the display.
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Clear the buffer, by default it has the adafruit logo saved.
  display.clearDisplay();
  display.display();

  // Print a start message.
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 9);
  display.cp437(true);

  display.println("Baby Fluke");
  display.display();
  delay(2000);

  // Prep Arduino SPI CS pin.
  pinMode(chipSelectPin, OUTPUT);
  digitalWrite(chipSelectPin, HIGH);

  // Initialize the network chip.
  initialize_enc();

  // Wait for link to come up.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 4);
  display.println("Cable unplugged or broken");
  display.display();
  while(!(read_phy(PHSTAT2) & PHSTAT2_LSTAT))
    ;

  // Notify user we're waiting for packet.
  display.clearDisplay();
  display.setCursor(0, 4);
  display.println("Waiting for Packet");
  display.display();
  while(true) {
    // //debugging.
    // if(rec_print_quit(buffer, next_packet_pointer, frame_size))
    //   break;

    // Decoding
    // Get a frame.
    frame_size = receive_packet(buffer, next_packet_pointer);

    // Determine packet and process it!
    while(true) {
      if ((buffer[20] << 8 | buffer[21]) == CDP_INDICATOR) {
        decodeCDP(buffer, frame_size);
      }
      else if ((buffer[12] << 8 | buffer[13]) == LLDP_INDICATOR) {
        decodeLLDP(buffer, frame_size);
      }
      else {
        break;
      }
    }

    // Decrement the packet counter.
    bfs(ECON2 & ADDRESS_MASK, ECON2_PKTDEC);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

// ===== START OF TEST ==============================================================
//     // Print Read Buffer Locations
//   Serial.println("Read Ptr Location:");
//   Serial.print(read_register(ERDPTH), BIN);
//   Serial.println(read_register(ERDPTL), BIN);

//   // Print Write Buffer Locations
//   Serial.println("Write Ptr Location:");
//   Serial.print(read_register(EWRPTH), BIN);
//   Serial.println(read_register(EWRPTL), BIN);

//   Serial.println("Updating Read Buffer Pointers...");
//   write_register(ERDPTH, 0x00);
//   write_register(ERDPTL, 0x00);

//   // Print Read Buffer Locations
//   Serial.println("Read Ptr Location:");
//   Serial.print(read_register(ERDPTH), BIN);
//   Serial.println(read_register(ERDPTL), BIN);

//   // Prepare the buffer for Writing to and Reading from the ENC28J60.
//   const uint16_t BUFSIZE = 128;
//   static uint8_t buffer[BUFSIZE];

//   // Write Buffer Memory
//   Serial.println("Writing to ENC...");
//   //char mytext[] = "Testing \"portability\" of code by running on another arduino.";
//   char mytext[] = "Board 1 - The fox jumped over the dog. There was also a frog";
//   for (uint8_t i = 0; i < sizeof(mytext); i++){
//     buffer[i] = mytext[i];
//   }
//   wbm(buffer, BUFSIZE);

//   // Print Read Buffer Values
//   rbm(buffer, BUFSIZE);
//   Serial.print("Read ");
//   Serial.print(BUFSIZE);
//   Serial.print(" addresses from buffer:");
//   for (int i=0; i<BUFSIZE;i++){
//     if (i % 16 == 0) { Serial.print("\n"); }
//     if (buffer[i] < 0x10) { Serial.print("0"); }
//     // Serial.print(*(buffer+i),HEX); // Print each character as HEX
//     Serial.print((char)buffer[i]);  // Print each character as CHAR
//     Serial.print(" ");
//   }
//   Serial.println("");

//   // Print Read Buffer Locations
//   Serial.println("Read Ptr Location:");
//   Serial.print(read_register(ERDPTH), BIN);
//   Serial.println(read_register(ERDPTL), BIN);

//   // Print Write Buffer Locations
//   Serial.println("Write Ptr Location:");
//   Serial.print(read_register(EWRPTH), BIN);
//   Serial.println(read_register(EWRPTL), BIN);

//   // Update PHY LED settings.
//   Serial.println("Read and Write PHLCON:");
//   Serial.println(read_phy(PHLCON), HEX);
//   write_phy(PHLCON, 0x3BA2); // Note the reserved bits.
//   Serial.println(read_phy(PHLCON), HEX);

  // // Test read pair and write pair.
  // Serial.println(read_register_pair(ERDPTL), HEX);
  // write_register_pair(ERDPTL, 0x1FFF);
  // Serial.println(read_register_pair(ERDPTL), HEX);

// =========== END OF TEST =========================================

  // Ethernet Frame - See Section 5.0 through 6.0 for Ethernet packet overview.
  // Destination MAC address - 6 bytes
  //    If the lowest bit of the first byte is set then this is a Multicast (multiple destination) packet.
  //    If the reserved Multicast address, FF-FF-FF-FF-FF-FF, is used then the packet will be broadcast to the whole network.
  //    If the lowest bit of the first byte is not set then this is a unicast (one destination) packet.
  //    The ENC28J60 can filter packets based on Multicast, Broadcast, or Unicast if you do not need to receive them.
  // Source MAC address - 6 bytes
  //    Must generate a unique address per node.
  //    First 3 bytes are an OUI handed out by IEEE, the last 3 bytes are at the discretion of the company that purchased the OUI.
  //    This must be written to the TX buffer. The MAADR registers' contents are only used for Unicast filtering and will not automatically populate into the TX packet.
  // Type/Length of data - 2 bytes
  //    Defines the protocol this data belongs to.
  //    If the field is 0x05DC (1500) or smaller then this becomes a length field specifying the amount of NON-PADDING data that follows in the data field.
  // Data - 46 to 1500 bytes.
  // Padding
  //    If the DA, SA, T/L, Data and Padding must not be smaller than 60 bytes for IEEE 802.3 specification.
  //    Set MACON3.PADCFG<2:0> to enable automatic zero padding.
  // Frame Check Sequence (CRC) - 4 bytes
  //    ERXFCON.CRCEN will discard packets with invalid CRCs.
  //    MACON3.PADCFG<2:0> can enable automatic CRC generation.
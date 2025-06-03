// Miscellaneous variables live here.

// SPI Chip Select.
#define chipSelectPin 10

// MAC address - Must be unique per node!
// First one is used in the initiliaze function.
// The second is for the send packet function.
#define MAC_ADDRESS 0x000012000000
#define MAC_ADDRESS_SIZE 6
const uint8_t MAC_ADDRESS_ARRAY[MAC_ADDRESS_SIZE] = {0x00, 0x00, 0x12, 0x00, 0x00, 0x00};

// Hold 1 packet in the TX buffer (everything not in RX buffer is deemed TX buffer)
// RX buffer gets 6662 bytes then.
#define RX_START_ADDRESS  1530
#define RX_END_ADDRESS    8191

// Maximum packet TX or RX length
#define MAXIMUM_FRAME_LENGTH 1518 // 1518 is the typical default.
#define MY_BUFFER_SIZE 500 // Save some space

// Per-packet override byte set to "DO NOT OVERRIDE"
#define TX_PACKET_OVERRIDE_BYTE 0x00
#define CONTROL_BYTE_SIZE 1

// Size of our (currently) fixed ethernet frames (68 bytes).
#define FRAME_LENGTH_DECIMAL 68
#define FRAME_LENGTH_SIZE 2
const uint8_t FRAME_LENGTH[FRAME_LENGTH_SIZE] = {0x00, 0x44};

// Message size in bytes.
#define MESSAGE_LENGTH 42

// Packet indicators.
#define CDP_INDICATOR   0x2000
#define LLDP_INDICATOR  0x88CC

// Frame offsets for decoding.
#define FRAME_HEADER_OFFSET 14
#define CRC_BYTES 4
#define FIRST_CDP_TLV_OFFSET 26
#define CDP_TLV_VALUE_OFFSET 4

// TLV types
#define CDP_DEVICE_ID             0x0001
#define CDP_ADDRESS               0x0002
#define CDP_PORT_ID               0x0003
#define CDP_SW_VERSION            0x0005
#define CDP_PLATFORM              0x0006
#define CDP_VTP_MANAGEMENT_DOMAIN 0x0009
#define CDP_NATIVE_VLAN_ID        0x000A
#define CDP_MANAGEMENT_ADDRESS    0x0016

// CDP Adressing info
#define PT_NLPID 0x01
#define NLPID_IP 0xCC

// Display screen info
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Input pin
#define CYCLE_BUTTON 9
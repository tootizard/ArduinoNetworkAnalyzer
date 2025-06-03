// ENC28j60 Register addresses and bits.
// Each address is 5 bits, leaving 3 bits of the byte unused.
// There are 4 banks though, so we can use 2 of the remaining 3 unused bits to represent the bank.
// The final bit will be used as a flag indicating that the register is a MAC or MII register. 
// Which requires additional logic in SPI commands.

// Bit masks
// 000xxxxx = Register address.
// 0xx00000 = Register bank.
// x0000000 = MAC/MII flag.
#define ADDRESS_MASK      0x1f
#define BANK_MASK         0x60
#define MACMII_FLAG_MASK  0x80

// Banks
#define BANK0 0x00
#define BANK1 0x20
#define BANK2 0x40
#define BANK3 0x60

// Bank 0
#define ERDPTL       (BANK0 | 0x00)
#define ERDPTH       (BANK0 | 0x01)
#define EWRPTL       (BANK0 | 0x02)
#define EWRPTH       (BANK0 | 0x03)
#define ETXSTL       (BANK0 | 0x04)
#define ETXSTH       (BANK0 | 0x05)
#define ETXNDL       (BANK0 | 0x06)
#define ETXNDH       (BANK0 | 0x07)
#define ERXSTL       (BANK0 | 0x08)
#define ERXSTH       (BANK0 | 0x09)
#define ERXNDL       (BANK0 | 0x0a)
#define ERXNDH       (BANK0 | 0x0b)
#define ERXRDPTL     (BANK0 | 0x0c)
#define ERXRDPTH     (BANK0 | 0x0d)
#define ERXWRPTL     (BANK0 | 0x0e)
#define ERXWRPTH     (BANK0 | 0x0f)
#define EDMASTL      (BANK0 | 0x10)
#define EDMASTH      (BANK0 | 0x11)
#define EDMANDL      (BANK0 | 0x12)
#define EDMANDH      (BANK0 | 0x13)
#define EDMADSTL     (BANK0 | 0x14)
#define EDMADSTH     (BANK0 | 0x15)
#define EDMACSL      (BANK0 | 0x16)
#define EDMACSH      (BANK0 | 0x17)
//#define --         (BANK0 | 0x18)
//#define --         (BANK0 | 0x19)
//#define Reserved   (BANK0 | 0x1a)
#define EIE          (BANK0 | 0x1b)
#define EIR          (BANK0 | 0x1c)
#define ESTAT        (BANK0 | 0x1d)
#define ECON2        (BANK0 | 0x1e)
#define ECON1        (BANK0 | 0x1f)

// Bank 1
#define EHT0         (BANK1 | 0x00)
#define EHT1         (BANK1 | 0x01)
#define EHT2         (BANK1 | 0x02)
#define EHT3         (BANK1 | 0x03)
#define EHT4         (BANK1 | 0x04)
#define EHT5         (BANK1 | 0x05)
#define EHT6         (BANK1 | 0x06)
#define EHT7         (BANK1 | 0x07)
#define EPMM0        (BANK1 | 0x08)
#define EPMM1        (BANK1 | 0x09)
#define EPMM2        (BANK1 | 0x0a)
#define EPMM3        (BANK1 | 0x0b)
#define EPMM4        (BANK1 | 0x0c)
#define EPMM5        (BANK1 | 0x0d)
#define EPMM6        (BANK1 | 0x0e)
#define EPMM7        (BANK1 | 0x0f)
#define EPMCSL       (BANK1 | 0x10)
#define EPMCSH       (BANK1 | 0x11)
//#define --         (BANK1 | 0x12)
//#define --         (BANK1 | 0x13)
#define EPMOL        (BANK1 | 0x14)
#define EPMOH        (BANK1 | 0x15)
//#define Reserved   (BANK1 | 0x16)
//#define Reserved   (BANK1 | 0x17)
#define ERXFCON      (BANK1 | 0x18)
#define EPKTCNT      (BANK1 | 0x19)
//#define Reserved   (BANK1 | 0x1a)
#define EIE          (BANK1 | 0x1b)
#define EIR          (BANK1 | 0x1c)
#define ESTAT        (BANK1 | 0x1d)
#define ECON2        (BANK1 | 0x1e)
#define ECON1        (BANK1 | 0x1f)

// Bank 2
#define MACON1       (BANK2 | 0x00 | MACMII_FLAG_MASK)
//#define Reserved   (BANK2 | 0x01)
#define MACON3       (BANK2 | 0x02 | MACMII_FLAG_MASK)
#define MACON4       (BANK2 | 0x03 | MACMII_FLAG_MASK)
#define MABBIPG      (BANK2 | 0x04 | MACMII_FLAG_MASK)
//#define --         (BANK2 | 0x05)
#define MAIPGL       (BANK2 | 0x06 | MACMII_FLAG_MASK)
#define MAIPGH       (BANK2 | 0x07 | MACMII_FLAG_MASK)
#define MACLCON1     (BANK2 | 0x08 | MACMII_FLAG_MASK)
#define MACLCON2     (BANK2 | 0x09 | MACMII_FLAG_MASK)
#define MAMXFLL      (BANK2 | 0x0a | MACMII_FLAG_MASK)
#define MAMXFLH      (BANK2 | 0x0b | MACMII_FLAG_MASK)
//#define Reserved   (BANK2 | 0x0c)
//#define Reserved   (BANK2 | 0x0d)
//#define Reserved   (BANK2 | 0x0e)
//#define --         (BANK2 | 0x0f)
//#define Reserved   (BANK2 | 0x10)
//#define Reserved   (BANK2 | 0x11)
#define MICMD        (BANK2 | 0x12 | MACMII_FLAG_MASK)
//#define --         (BANK2 | 0x13)
#define MIREGADR     (BANK2 | 0x14 | MACMII_FLAG_MASK)
//#define Reserved   (BANK2 | 0x15)
#define MIWRL        (BANK2 | 0x16 | MACMII_FLAG_MASK)
#define MIWRH        (BANK2 | 0x17 | MACMII_FLAG_MASK)
#define MIRDL        (BANK2 | 0x18 | MACMII_FLAG_MASK)
#define MIRDH        (BANK2 | 0x19 | MACMII_FLAG_MASK)
//#define Reserved   (BANK2 | 0x1a)
#define EIE          (BANK2 | 0x1b)
#define EIR          (BANK2 | 0x1c)
#define ESTAT        (BANK2 | 0x1d)
#define ECON2        (BANK2 | 0x1e)
#define ECON1        (BANK2 | 0x1f)

// Bank 3
#define MAADR5       (BANK3 | 0x00 | MACMII_FLAG_MASK)
#define MAADR6       (BANK3 | 0x01 | MACMII_FLAG_MASK)
#define MAADR3       (BANK3 | 0x02 | MACMII_FLAG_MASK)
#define MAADR4       (BANK3 | 0x03 | MACMII_FLAG_MASK)
#define MAADR1       (BANK3 | 0x04 | MACMII_FLAG_MASK)
#define MAADR2       (BANK3 | 0x05 | MACMII_FLAG_MASK)
#define EBSTSD       (BANK3 | 0x06)
#define EBSTCON      (BANK3 | 0x07)
#define EBSTCSL      (BANK3 | 0x08)
#define EBSTCSH      (BANK3 | 0x09)
#define MISTAT       (BANK3 | 0x0a | MACMII_FLAG_MASK)
//#define --         (BANK3 | 0x0b)
//#define --         (BANK3 | 0x0c)
//#define --         (BANK3 | 0x0d)
//#define --         (BANK3 | 0x0e)
//#define --         (BANK3 | 0x0f)
//#define --         (BANK3 | 0x10)
//#define --         (BANK3 | 0x11)
#define EREVID       (BANK3 | 0x12)
//#define --         (BANK3 | 0x13)
//#define --         (BANK3 | 0x14)
#define ECOCON       (BANK3 | 0x15)
//#define Reserved   (BANK3 | 0x16)
#define EFLOCON      (BANK3 | 0x17)
#define EPAUSL       (BANK3 | 0x18)
#define EPAUSH       (BANK3 | 0x19)
//#define Reserved   (BANK3 | 0x1a)
#define EIE          (BANK3 | 0x1b)
#define EIR          (BANK3 | 0x1c)
#define ESTAT        (BANK3 | 0x1d)
#define ECON2        (BANK3 | 0x1e)
#define ECON1        (BANK3 | 0x1f)

// PHYS registers
#define PHCON1  0x00
#define PHSTAT1 0x01
#define PHID1   0x02
#define PHID2   0x03
#define PHCON2  0x10
#define PHSTAT2 0x11
#define PHIE    0x12
#define PHIR    0x13
#define PHLCON  0x14

// Register Bits
#define ECON1_TXRTS 0x08
#define ECON1_RXEN  0x04
#define ECON1_BSEL1 0x02
#define ECON1_BSEL0 0x01

#define ECON2_PKTDEC 0x40

#define ESTAT_RXBUSY 0x04
#define ESTAT_TXABRT 0x02
#define ESTAT_CLKRDY 0x01

#define EIE_PKTIE 0x40
#define EIE_TXIE  0x08

#define EIR_PKTIF 0x40
#define EIR_TXIF  0x08

#define MICMD_MIIRD 0x01

#define MISTAT_BUSY 0x01

#define MACON1_MARXEN   0x01
#define MACON1_PASSALL  0x02
#define MACON1_RXPAUS   0x04
#define MACON1_TXPAUS   0x08

#define MACON3_FULDPX    0x01
#define MACON3_FMLNEN    0x02
#define MACON3_HFRMEN    0x04
#define MACON3_PHDREN    0x08
#define MACON3_TXCRCEN   0x10
#define MACON3_PADCFG0   0x20
#define MACON3_PADCFG1   0x30
#define MACON3_PADCFG2   0x40

#define MACON4_DEFER  0x40

#define PHSTAT2_LSTAT 0x0400
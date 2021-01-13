/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _SH1106_H_
#define _SH1106_H_

#define SH1106_I2C_SCL_CLOCK  800000UL
#define SH1106_I2C_ADDR 0b01111000
#define SH1106_COMMAND 0x00
#define SH1106_DATA 0x40

#define SH1106_COLS_USED 128
#define SH1106_PAGES 8

#ifndef LCD_SSD1306
  #define SH1106_COLS_TOTAL 130
#else 
  #define SH1106_COLS_TOTAL SH1106_COLS_USED
#endif

const uint8_t SH1106_initCommands[] = {
    0xAE,0xDE,0x80,0xA8,
    0x3F,0xD3,0x00,0x40,
    0x8D,0x14,0x20,0x00,
    0xA1,0xC8,0xDA,0x12,
    0x81,0xCF,0xD9,0xF1,
    0xDB,0x40,0xA4,0xA6,
};
  
template<class I2C_TYPE>
class SH1106 {
  I2C_TYPE &i2c;
  const uint16_t screenBufferSize;
public:
  enum AddressingMode {
    HorizontalAddressingMode = 0x00,
    VerticalAddressingMode = 0x01,
    PageAddressingMode = 0x02,
  };
  SH1106(I2C_TYPE &i2c, const uint16_t screenBufferSize): 
    i2c(i2c), screenBufferSize(screenBufferSize) {
  }

  void begin() {
    uint8_t col = 0;
    uint8_t page = 0;

        i2c.deinit();
    i2c.init(SH1106_I2C_SCL_CLOCK);

    for(int i=0;i<sizeof(SH1106_initCommands);i++){
       sendc(SH1106_initCommands[i]);
    }
  }

  void fillScreen(const uint8_t* buffer) {
    reinit();
    sendd(buffer, screenBufferSize);
  }
  void fillScreen(const uint8_t* buffer, const uint16_t size, const uint8_t stride = 0) { //stride is not supported
    reinit();
    sendd(buffer, size);
  }
  void setInverse(const bool v) {
    inverted = v;
    reinit();
  }
  //Only PageAddressingMode works for SH1106
  //Here we're emulating HorizontalAddressingMode programmatically
  void setAddressingMode(const AddressingMode addressingMode) {} 
  
private:
  
  /* lcd control */
  void reinit() {

//  sendc(SSD1306_DISPLAYON);//--turn on oled panel
    sendc(0xAF); //display on
    sendc(inverted? 0xA7 : 0xA6); //inversion
  }
  
  /* send bytes */
  void sendc(const uint8_t cmd) {
    i2c.start(SH1106_I2C_ADDR);
    i2c.write(SH1106_COMMAND);
    i2c.write(cmd);
    i2c.stop();
  }
  void sendc(const uint8_t cmd1, const uint8_t cmd2) {
    i2c.start(SH1106_I2C_ADDR);
    i2c.write(SH1106_COMMAND);
    i2c.write(cmd1);
    i2c.write(cmd2);
    i2c.stop();
  }
  void sendc(const uint8_t cmd1, const uint8_t cmd2, const uint8_t cmd3) {
    i2c.start(SH1106_I2C_ADDR);
    i2c.write(SH1106_COMMAND);
    i2c.write(cmd1);
    i2c.write(cmd2);
    i2c.write(cmd3);
    i2c.stop();
  }
  void sendd(const uint8_t* d, uint16_t sz) {  
    const uint8_t blackPx = inverted? 0xFF : 0;
    while(sz) {  //page cycle
      sendc(0xB0 + page); //set page address to i (0..7)
      sendc(0x00 + (col&0x0F)); //Sets 4 lower bits of column address 
      sendc(0x10 + (col>>4)); //Sets 4 higher bits of column address 
      
      i2c.start(SH1106_I2C_ADDR);
      i2c.write(SH1106_DATA);
      while(sz) { //column cycle
        i2c.write(*d++);
        --sz;
        if(++col >= SH1106_COLS_USED) {
          while(col++ < SH1106_COLS_TOTAL) i2c.write(blackPx);
          col = 0;
          break;
        }
      }
      i2c.stop();
    
      if(col == 0 && ++page >= SH1106_PAGES) page = 0;
    }     
  }

  uint8_t col = 0;
  uint8_t page = 0;
  bool inverted = false;
};


#endif

#ifndef __lcd_hpp__
#define __lcd_hpp__
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>

// frame buffer size
#define ROWS 4
#define COLUMNS 20

// data bits on PD4..PD7
// E signal on PB0
// RS signal on PB1
#define E_HIGH() (PORTB|=1)
#define E_LOW() (PORTB&=(~1))
#define RS_HIGH() (PORTB|=2)
#define RS_LOW() (PORTB&=(~2))
#define DATA_OUTH(x) (PORTD=(PORTD&0x0f)|((x&0xf0)))
#define DATA_OUTL(x) (PORTD=(PORTD&0x0f)|((x<<4)&0xf0))

class LCD
{
private:

  // we have plenty of memory to hold entire visible frame for
  // easier scrolling
  uint8_t framebuffer[ROWS*COLUMNS];
  uint8_t cx,cy,showcursor;
  
  // send data or command in two nibbles
  void output(uint8_t d)
  {
    E_HIGH();
    DATA_OUTH(d); E_LOW(); E_HIGH();
    DATA_OUTL(d); E_LOW(); E_HIGH();
    _delay_us(50);
  }
  
  void cmd(uint8_t d) { RS_LOW(); output(d); }
  void data(uint8_t d) { RS_HIGH(); output(d); }
  
  void scrollup()
  {
    // shift data by one row in frame buffer, and erase last row
    memcpy(framebuffer,framebuffer+COLUMNS,ROWS*COLUMNS-COLUMNS);
    memset(framebuffer+ROWS*COLUMNS-COLUMNS,' ',COLUMNS);
    // now copy the frame buffer to screen
    uint8_t r,c;
    for (r=0;r<ROWS;r++) {
      uint8_t adr=(r&1)*64; // first two rows
      if (r>1)
        adr+=COLUMNS;
      cmd(adr|0x80);  // cursor to start of row
      for (c=0;c<COLUMNS;c++) {
        data(framebuffer[r*COLUMNS+c]);
      }
    }
    cmd((cy*64+cx)|0x80);
  }
  
public:
  // constructor is not a good place for initialization
  // as the I/O is probably not configured yet
  LCD() { }
  
  // clear screen and home cursor
  void clear() {
    cmd(0x0c);
    cmd(0x01);
    _delay_ms(2);
    cx=cy=0;
    memset(framebuffer,' ',sizeof(framebuffer));
    if (showcursor)
      cmd(0x0f);
  }

  // move cursor to home
  void home()
  {
    cx=cy=0;
    cmd(0x02);
    if (showcursor)
      cmd(0x0f);
  }

  // write one character at cursor location
  void printc(uint8_t c) {
    switch (c) {
      case '\b':  // backspace moves cursor left
        if (cx>0) {
          cursorpos(cx-1,cy);
        }
        break;
      case '\v': // vertical tab homes cursor
        home();
        break;
      case '\t': // horizontal tab moves cursor right
        if (cx<=COLUMNS) {
          cursorpos(cx+1,cy);
        }
        break;
      case '\f': // form feed clears screen and homes cursor
        clear();
        break;
      case '\n': // newline moves cursor down, on last line scrolls screen up
        if (cy>=ROWS-1)
          scrollup();
        else
          cursorpos(cx,cy+1);
        break;
      case '\r': // carriage return returns cursor to start of row
        cursorpos(0,cy);
        break;
      default:
        if (cx>=COLUMNS) {
          if (cy>=(ROWS-1))
            scrollup();
          else
            cy++;
          if (cy>(ROWS-1))
            cy=ROWS-1;
          cursorpos(0,cy);
        }
        framebuffer[cy*COLUMNS+cx]=c;
        data(c);
        cx++;
        break;
    }
  }

  // enable or disable blinking cursor
  void cursoronoff(uint8_t enabled)
  { 
    showcursor=enabled;
    if (enabled)
      cmd(0x0f);
    else
      cmd(0x0c);
  }
  
  // reposition cursor, coordinates start from 0
  void cursorpos(uint8_t x,uint8_t y) {
    cx=x;
    cy=y;
    uint8_t adr=(y&1)*64+x; // first two rows
    if (y>1)
      adr+=COLUMNS;
    cmd(adr|0x80);
  }
  
  // reset the lcd controller, and set to 4 bit mode
  void reset()
  {
    RS_HIGH();
    E_HIGH();
    DATA_OUTL(0x0f); _delay_ms(20);
    DATA_OUTL(0x03); RS_LOW(); E_LOW(); _delay_ms(15); E_HIGH();
    E_LOW(); _delay_ms(5); E_HIGH();
    E_LOW(); _delay_ms(5); E_HIGH();
    DATA_OUTL(0x02); E_LOW(); _delay_ms(5); E_HIGH();
    cmd(0x28); // 4 bit mode, 2 line, 5x7 font
    cmd(0x06); // autoincrement, no display shift
    clear();
    showcursor=0;
  }

};
#endif

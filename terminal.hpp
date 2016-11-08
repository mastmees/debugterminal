/*
The MIT License (MIT)

Copyright (c) 2016 Madis Kaal <mast@nomad.ee>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __terminal_hpp__
#define __terminal_hpp__
#include <avr/io.h>

#define BAUDRATE 1200

class Terminal
{
public:
  Terminal()
  {
  }

  void putc(uint8_t c)
  {
    if (c=='\n')
      putc('\r');
    while ( !(UCSR0A & _BV(UDRE0)) );
    UDR0=c;
  }
  
  void puts(const char *s)
  {
    while (*s) {
      putc(*s++);
    }
  }
  
  uint8_t ready()
  {
    return (UCSR0A & _BV(RXC0));
  }
  
  uint8_t getch()
  {
    return UDR0;
  }
  
  void putn(int32_t n)
  {
    if (n<0)
      n=0-n;
    if (n>9)
      putn(n/10);
    putc((n%10)+'0');
  }

  void clear()
  {
    putc('\f');
  }
  
  void home()
  {
    putc('\v');
  }

  void init()
  {
    UCSR0A|=_BV(U2X0); // double-speed
    UBRR0H=(F_CPU/(8UL*BAUDRATE)-1)>>8;
    UBRR0L=(F_CPU/(8UL*BAUDRATE)-1)&0xff;
    UCSR0B=_BV(TXEN0)|_BV(RXEN0);   // enable rx,tx
    clear();
  }
  
    
};

#endif

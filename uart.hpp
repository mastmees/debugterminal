#ifndef __uart_hpp__
#define __uart_hpp__
#include <avr/io.h>

class UART
{
  volatile uint8_t rbuf[8],head,tail,count;
  
public:
  UART() : head(0),tail(0),count(0)
  {
  }
  
  void received(uint8_t c)
  {
    if (count<(sizeof(rbuf)/sizeof(rbuf[0]))) {
      count++;
      rbuf[head++]=c;
      head%=(sizeof(rbuf)/sizeof(rbuf[0]));
    }
  }
  
  uint8_t ready()
  {
    return count;
  }
  
  uint8_t read()
  {
  uint8_t c;
    if (count) {
      c=rbuf[tail++];
      tail%=(sizeof(rbuf)/sizeof(rbuf[0]));
      count--;
      return c;
    }
    return 0;
  }
  
  uint8_t empty()
  {
    return ((UCSR0A & _BV(UDRE0)));
  }

  void send(uint8_t c)
  {
    while ((UCSR0A & _BV(UDRE0))==0);
    UDR0=c;
  }
  
  void initialize(uint32_t baudrate)
  {
    head=tail=count=0;
    UBRR0H=((F_CPU/(16UL*baudrate))-1)>>8;
    UBRR0L=((F_CPU/(16UL*baudrate))-1)&0xff;
    UCSR0B=_BV(RXEN0)|_BV(TXEN0)|_BV(RXCIE0);
  }
  
};

#endif

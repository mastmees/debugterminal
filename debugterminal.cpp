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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <string.h>

#include "lcd.hpp"
#include "keypad.hpp"
#include "uart.hpp"

#define TICKPERIOD 256-(uint8_t)(F_CPU/(1024L*80L)) // 80ms keypad scan interval

LCD display;
KeyPad keypad;
UART uart;

uint32_t baudrates[]=
{
57600,   // 000 - B4,B5 jumpers on          ||:
115200,  // 001 - all jumpers on            |||
19200,   // 010 - B5 jumper on              |::
38400,   // 011 - B3,B5 jumpers on          |:|
4800,    // 100 - B4 jumper on              :|:
9600,    // 101 - B3,B4 jumpers on          :||
1200,    // 110 - default with no jumpers   :::
2400     // 111 - B3 jumper on              ::|
};

ISR(WDT_vect)
{
}

ISR(TIMER0_OVF_vect)
{
  // reset timer for next interrupt
  TCNT0=TICKPERIOD;
}

ISR(USART_RX_vect)
{
  uart.received(UDR0);
}

/*
I/O configuration
-----------------
I/O pin                               direction    DDR  PORT
PC0 unused                            output       0    0
PC1 unused                            output       0    0
PC2 keypad col 4                      input        0    1
PC3 keypad col 3                      input        0    1
PC4 keypad col 2                      input        0    1
PC5 keypad col 1                      input        0    1

PD0 RxD                               input        0    0
PD1 TxD                               output       1    0
PD2 unused                            output       1    0
PD3 unused                            output       1    0
PD4 display D4/keypad row 1           output       1    0
PD5 display D5/keypad row 2           output       1    0
PD6 display D6/keypad row 3           output       1    0
PD7 display D7/keypad row 4           output       1    0

PB0 display E                         output       1    0
PB1 display RS                        output       1    0
PB2 unused                            output       1    0
PB3 jumper1                           input        0    0
PB4 jumper2                           input        0    1
PB5 jumper3                           input        0    1
*/
int main(void)
{
  MCUSR=0;
  MCUCR=0;
  // I/O directions
  DDRC=0x00;
  DDRD=0xfe;
  DDRB=0x07;
  // initial state
  PORTC=0x3c;
  PORTD=0x00;
  PORTB=0x30;
  //
  // configure timer0 for periodic interrupts
  // for keypad scanning
  TCCR0B=5; // timer clock prescaler 1024
  TIMSK0=1;
  TCNT0=TICKPERIOD;
  //
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  // configure watchdog to interrupt&reset, 4 sec timeout
  WDTCSR|=0x18;
  WDTCSR=0xe8;
  //
  display.reset();
  display.cursoronoff(1);
  //
  uint8_t c=(PINB>>3)&7;
  uart.initialize(baudrates[c]);
  //
  sei();
  while (1) {
    sleep_cpu();
    // sleep ends with either timer or serial port interrupt
    // in either case reset watchdog, scan keypad, and
    // output received data
    wdt_reset();
    WDTCSR|=0x40;
    keypad.scan();
    if (keypad.ready() && uart.empty()) {
      uart.send(keypad.getch());
    }
    if (uart.ready()) {
      display.printc(uart.read());
    }
  }
}


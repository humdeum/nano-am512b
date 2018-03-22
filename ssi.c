#include <stdio.h>

#include "ssi.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

int ssi_read(int b)
{
    int i, sangle=0, data[b];

    PORTD = (1<<DDD5);
    _delay_us(UDELAY);
    PORTD = (0<<DDD5);
    _delay_us(UDELAY); // begin read

    for (i=0; i<9; i++)
    {
      PORTD = (1<<DDD5);
      _delay_us(UDELAY);

      data[i] = (PIND & (1<<DDD6)) == (1<<DDD6) ? 1 : 0;

      PORTD = (0<<DDD5);
      _delay_us(UDELAY);
    }

    _delay_us(TM); // end read

    printf("Serial: ");
    for (i=0; i<9; i++)
    {
      printf("%d", data[i]);
      sangle += data[i] << (8-i);
    }
    return sangle;
}

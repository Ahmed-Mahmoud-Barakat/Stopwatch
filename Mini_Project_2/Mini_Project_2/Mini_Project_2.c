#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char sec = 0, min = 0, hr = 0;

unsigned char sec_Ones = 0, sec_Tens = 0, min_Ones = 0, min_Tens = 0, hr_Ones = 0, hr_Tens = 0;

unsigned char Timer_flag = 0;

unsigned char Toggle_flag = 0;

ISR(TIMER1_COMPA_vect)
{
	Timer_flag =1;
}

ISR(INT0_vect)
{
	sec_Ones = 0;
	sec_Tens = 0;
	min_Ones = 0;
	min_Tens = 0;
	hr_Ones = 0;
	hr_Tens = 0;
	sec = 0;
	min = 0;
	hr = 0;
	TCNT1 = 0;
}

ISR(INT1_vect)
{
	TIMSK &=~(1<<OCIE1A);
	TCNT1 = 0;
}

ISR(INT2_vect)
{
	TIMSK |= (1<<OCIE1A);
	TCNT1 = 0;
}

void Timer1_Init_CTC_Mode(void)
{
	TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<CS12);
	TCCR1A = (1<<FOC1A);
	OCR1A = 15625;
	TIMSK |= (1<<OCIE1A);
}

void INT0_Init(void)
{
	DDRD  &= (~(1<<PD2));               // Configure INT0/PD2 as input pin
	MCUCR |= (1<<ISC11);				// Trigger INT0 with the falling edge
	MCUCR &= ~(1<<ISC10);
	GICR  |= (1<<INT0);                 // Enable external interrupt pin INT0
}

void INT1_Init(void)
{
	DDRD  &= (~(1<<PD3));  // Configure INT1/PD3 as input pin
	MCUCR |= (1<<ISC00) | (1<<ISC01);   // Trigger INT0 with the raising edge
	GICR  |= (1<<INT1);    // Enable external interrupt pin INT1
}
void INT2_Init(void)
{
	DDRB  &= (~(1<<PB2));   // Configure INT2/PB2 as input pin
	MCUCR |= (1<<ISC11);	 // Trigger INT0 with the falling edge
	MCUCR &= ~(1<<ISC10);
	GICR  |= (1<<INT2);	 // Enable external interrupt pin INT2
}

void increment(void)
{
	TCNT1 = 0;
	PORTD &=~(1<<PD0);
	sec++;
	if(sec > 59)
	{
		sec = 0;
		min++;
	}
	if (min > 59)
	{
		min = 0;
		hr++;
	}
	if (hr > 99)
	{
		hr = 0;
	}
	sec_Tens = sec/10;
	sec_Ones = sec - sec_Tens*10 ;

	min_Tens = min/10;
	min_Ones = min - min_Tens*10 ;

	hr_Tens = hr/10;
	hr_Ones = hr - hr_Tens*10 ;

	Timer_flag = 0;
}
void decrement(void)
{
	TCNT1 = 0;
	if(sec==0 && min!=0)
	{
		min--;
		sec=59;
	}
	else if( sec == 0 && min == 0 && hr != 0)
	{
		hr--;
		min=59;
		sec=59;
	}
	else if(sec==0 && min==0 && hr==0)
	{
		sec=0;
		PORTD |=(1<<PD0);
	}
	else
	{
		sec--;
	}

	sec_Tens = sec/10;
	sec_Ones = sec - sec_Tens*10 ;

	min_Tens = min/10;
	min_Ones = min - min_Tens*10 ;

	hr_Tens = hr/10;
	hr_Ones = hr - hr_Tens*10 ;

	Timer_flag = 0;

}

void Display(void)
{
	PORTA &=~(1<<PA4)&~(1<<PA3)&~(1<<PA2)&~(1<<PA1)&~(1<<PA0);
	PORTA |=(1<<PA5);
	PORTC = sec_Ones;
	_delay_ms(2);

	PORTA &=~(1<<PA5)&~(1<<PA3)&~(1<<PA2)&~(1<<PA1)&~(1<<PA0);
	PORTA |=(1<<PA4);
	PORTC = sec_Tens;
	_delay_ms(2);

	PORTA &=~(1<<PA4)&~(1<<PA5)&~(1<<PA2)&~(1<<PA1)&~(1<<PA0);
	PORTA |=(1<<PA3);
	PORTC = min_Ones;
	_delay_ms(2);

	PORTA &=~(1<<PA4)&~(1<<PA3)&~(1<<PA5)&~(1<<PA1)&~(1<<PA0);
	PORTA |=(1<<PA2);
	PORTC = min_Tens;
	_delay_ms(2);

	PORTA &=~(1<<PA4)&~(1<<PA3)&~(1<<PA2)&~(1<<PA5)&~(1<<PA0);
	PORTA |=(1<<PA1);
	PORTC = hr_Ones;
	_delay_ms(2);

	PORTA &=~(1<<PA4)&~(1<<PA3)&~(1<<PA2)&~(1<<PA1)&~(1<<PA5);
	PORTA |=(1<<PA0);
	PORTC = hr_Tens;
	_delay_ms(2);
}

int main(void)
{
	DDRC |= 0x0F;
	PORTC &= 0xF0;
	DDRA |=0x3F;
	DDRD |=(1<<PD5)|(1<<PD4)|(1<<PD0);
	PORTD &=~(1<<PD5);
	PORTD &=~(1<<PD0);
	PORTD |=(1<<PD4);
	SREG  |= (1<<7);
	DDRB &=~0xFF;
	PORTB |= (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7);
	PORTD |= (1<<PD2);
	Timer1_Init_CTC_Mode();
	INT0_Init();
	INT1_Init();
	INT2_Init();

	while(1)
	{

		Display();

		if(!(PINB & (1<<PB7)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB7)))
			{
				Toggle_flag ^= 1;
				PORTD ^= (1<<PD5);
				PORTD ^= (1<<PD4);
				while(!(PINB & (1<<PB7)))
				{
					Display();
				}
			}
		}


		if(!(PINB & (1<<PB5)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB5)))
			{
				if(sec==0)
				{
					sec=0;
				}
				else
				{
					sec--;
				}
				sec_Tens = sec/10;
				sec_Ones = sec - sec_Tens*10 ;

			}
			while(!(PINB & (1<<PB5)))
			{
				Display();

			}
		}

		if(!(PINB & (1<<PB6)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB6)))
			{
				sec++;
				if(sec > 59)
				{
					sec = 59;
				}

				sec_Tens = sec/10;
				sec_Ones = sec - sec_Tens*10 ;

			}
			while(!(PINB & (1<<PB6)))
			{
				Display();

			}

		}

		if(!(PINB & (1<<PB3)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB3)))
			{
				if(min==0)
				{
					min=0;
				}
				else
				{
					min--;
				}

				min_Tens = min/10;
				min_Ones = min - min_Tens*10 ;
			}
			while(!(PINB & (1<<PB3)))
			{
				Display();

			}

		}

		if(!(PINB & (1<<PB4)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB4)))
			{
				min++;
				if (min > 59)
				{
					min = 59;
				}

				min_Tens = min/10;
				min_Ones = min - min_Tens*10 ;

			}
			while(!(PINB & (1<<PB4)))
			{
				Display();

			}

		}

		if(!(PINB & (1<<PB0)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB0)))
			{
				if(hr==0)
				{
					hr=0;
				}
				else
				{
					hr--;

				}
				hr_Tens = hr/10;
				hr_Ones = hr - hr_Tens*10 ;
			}
			while(!(PINB & (1<<PB0)))
			{
				Display();

			}

		}

		if(!(PINB & (1<<PB1)))
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB1)))
			{
				hr++;
				if (hr > 99)
				{
					hr = 99;
				}

				hr_Tens = hr/10;
				hr_Ones = hr - hr_Tens*10 ;

			}
			while(!(PINB & (1<<PB1)))
			{
				Display();

			}

		}

		if (Timer_flag == 1)
		{
			if(Toggle_flag==0)
			{
				increment();
			}
			else
			{
				decrement();
			}
		}
	}
}

/*
 * main.cpp
 *
 *  Created on: Jul 11, 2014
 *      Author: Jonathan Whitfield
 */



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdbool.h>

#define ACTIVE_HIGH

#define EXTERNAL_NUM_INTERRUPTS 2
#define EXTERNAL_INT_0 0
#define EXTERNAL_INT_1 1
#define EXTERNAL_INT_2 2

#define LED_PORT 	PORTB
#define LED_INPUT	PINB
#define LED_DDR		DDRB
#define LED_PIN		PB5

#define CHANGE 1
#define FALLING 2
#define RISING 3

#ifdef ACTIVE_HIGH
#define LED_ON()	LED_PORT |= _BV(LED_PIN);
#define LED_OFF()	LED_PORT &= ~_BV(LED_PIN);
#endif

#ifdef ACTIVE_LOW
#define LED_ON()	LED_PORT &= ~_BV(LED_PIN);
#define LED_OFF()	LED_PORT |= _BV(LED_PIN);
#endif

void init();
void startTimer(uint16_t counts);
void resetTimer(uint16_t counts);
void detectedPulse();
void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode);

volatile uint16_t countsToWait = 20000;/*20 ms @ 8MHz*/

volatile bool car = false;


int main(void){

	init();

	for(;;);

	return 0;
}


void init(){
	DDRD &= ~_BV(PIND2);//PD2 as input
	//PORTD |= _BV(PIND2);//Pull up enabled
	LED_DDR |= _BV(LED_PIN);//Output for light

	for(int i=0;i<10;i++){
		if(!(LED_INPUT & _BV(LED_PIN))){/*If the LED is off*/
			/*Turn on the staging light*/
			LED_ON();
		}else{
			LED_OFF();
		}
		_delay_ms(1000);
	}

	attachInterrupt(EXTERNAL_INT_0, detectedPulse, FALLING);
	startTimer(countsToWait);
	sei();
}

void startTimer(uint16_t counts){
	TCCR1A = _BV(COM1B1);
	TCCR1B = _BV(CS11)| _BV(WGM12);//Clock div by 8
	TCNT1 = 0x00;
	OCR1B = counts;
	TIMSK1 = _BV(OCIE1B);
}

void resetTimer(uint16_t counts){
	TCNT1 = 0x00;
	OCR1B = counts;
	TIMSK1 = _BV(OCIE1B);
}

void detectedPulse(){
	/*we detected a pulse, reset the timer*/
	resetTimer(countsToWait);
	car = false;
#ifdef ACTIVE_HIGH
	if((LED_INPUT & _BV(LED_PIN))){/*If the LED is on*/
#endif
#ifdef ACTIVE_LOW
		if(!(LED_INPUT & _BV(LED_PIN))){/*If the LED is on*/
#endif
			/*Turn off the staging light*/
			LED_OFF();
		}
	}

	ISR(TIMER1_COMPB_vect){
		/*Executes when the timer is not reset = a car is present*/
		car = true;
		/*Turn on the staging light*/
		LED_ON();
		resetTimer(countsToWait);
	}



	typedef void (*voidFuncPtr)(void);

	static volatile voidFuncPtr intFunc[EXTERNAL_NUM_INTERRUPTS];
	// volatile static voidFuncPtr twiIntFunc;

	void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
		if(interruptNum < EXTERNAL_NUM_INTERRUPTS) {
			intFunc[interruptNum] = userFunc;

			// Configure the interrupt mode (trigger on low input, any change, rising
			// edge, or falling edge). The mode constants were chosen to correspond
			// to the configuration bits in the hardware register, so we simply shift
			// the mode into place.

			// Enable the interrupt.

			switch (interruptNum) {
#if defined(__AVR_ATmega32U4__)
			// I hate doing this, but the register assignment differs between the 1280/2560
			// and the 32U4. Since avrlib defines registers PCMSK1 and PCMSK2 that aren't
			// even present on the 32U4 this is the only way to distinguish between them.
			case 0:
				EICRA = (EICRA & ~((1<<ISC00) | (1<<ISC01))) | (mode << ISC00);
				EIMSK |= (1<<INT0);
				break;
			case 1:
				EICRA = (EICRA & ~((1<<ISC10) | (1<<ISC11))) | (mode << ISC10);
				EIMSK |= (1<<INT1);
				break;
			case 2:
				EICRA = (EICRA & ~((1<<ISC20) | (1<<ISC21))) | (mode << ISC20);
				EIMSK |= (1<<INT2);
				break;
			case 3:
				EICRA = (EICRA & ~((1<<ISC30) | (1<<ISC31))) | (mode << ISC30);
				EIMSK |= (1<<INT3);
				break;
			case 4:
				EICRB = (EICRB & ~((1<<ISC60) | (1<<ISC61))) | (mode << ISC60);
				EIMSK |= (1<<INT6);
				break;
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
			case 2:
				EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
				EIMSK |= (1 << INT0);
				break;
			case 3:
				EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
				EIMSK |= (1 << INT1);
				break;
			case 4:
				EICRA = (EICRA & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
				EIMSK |= (1 << INT2);
				break;
			case 5:
				EICRA = (EICRA & ~((1 << ISC30) | (1 << ISC31))) | (mode << ISC30);
				EIMSK |= (1 << INT3);
				break;
			case 0:
				EICRB = (EICRB & ~((1 << ISC40) | (1 << ISC41))) | (mode << ISC40);
				EIMSK |= (1 << INT4);
				break;
			case 1:
				EICRB = (EICRB & ~((1 << ISC50) | (1 << ISC51))) | (mode << ISC50);
				EIMSK |= (1 << INT5);
				break;
			case 6:
				EICRB = (EICRB & ~((1 << ISC60) | (1 << ISC61))) | (mode << ISC60);
				EIMSK |= (1 << INT6);
				break;
			case 7:
				EICRB = (EICRB & ~((1 << ISC70) | (1 << ISC71))) | (mode << ISC70);
				EIMSK |= (1 << INT7);
				break;
#else
			case 0:
#if defined(EICRA) && defined(ISC00) && defined(EIMSK)
				EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
				EIMSK |= (1 << INT0);
#elif defined(MCUCR) && defined(ISC00) && defined(GICR)
				MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
				GICR |= (1 << INT0);
#elif defined(MCUCR) && defined(ISC00) && defined(GIMSK)
				MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
				GIMSK |= (1 << INT0);
#else
#error attachInterrupt not finished for this CPU (case 0)
#endif
				break;

			case 1:
#if defined(EICRA) && defined(ISC10) && defined(ISC11) && defined(EIMSK)
				EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
				EIMSK |= (1 << INT1);
#elif defined(MCUCR) && defined(ISC10) && defined(ISC11) && defined(GICR)
				MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
				GICR |= (1 << INT1);
#elif defined(MCUCR) && defined(ISC10) && defined(GIMSK) && defined(GIMSK)
				MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
				GIMSK |= (1 << INT1);
#else
#warning attachInterrupt may need some more work for this cpu (case 1)
#endif
				break;

			case 2:
#if defined(EICRA) && defined(ISC20) && defined(ISC21) && defined(EIMSK)
				EICRA = (EICRA & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
				EIMSK |= (1 << INT2);
#elif defined(MCUCR) && defined(ISC20) && defined(ISC21) && defined(GICR)
				MCUCR = (MCUCR & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
				GICR |= (1 << INT2);
#elif defined(MCUCR) && defined(ISC20) && defined(GIMSK) && defined(GIMSK)
				MCUCR = (MCUCR & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
				GIMSK |= (1 << INT2);
#endif
				break;
#endif
			}
		}
	}

	void detachInterrupt(uint8_t interruptNum) {
		if(interruptNum < EXTERNAL_NUM_INTERRUPTS) {
			// Disable the interrupt. (We can't assume that interruptNum is equal
			// to the number of the EIMSK bit to clear, as this isn't true on the
			// ATmega8. There, INT0 is 6 and INT1 is 7.)
			switch (interruptNum) {
#if defined(__AVR_ATmega32U4__)
			case 0:
				EIMSK &= ~(1<<INT0);
				break;
			case 1:
				EIMSK &= ~(1<<INT1);
				break;
			case 2:
				EIMSK &= ~(1<<INT2);
				break;
			case 3:
				EIMSK &= ~(1<<INT3);
				break;
			case 4:
				EIMSK &= ~(1<<INT6);
				break;
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
			case 2:
				EIMSK &= ~(1 << INT0);
				break;
			case 3:
				EIMSK &= ~(1 << INT1);
				break;
			case 4:
				EIMSK &= ~(1 << INT2);
				break;
			case 5:
				EIMSK &= ~(1 << INT3);
				break;
			case 0:
				EIMSK &= ~(1 << INT4);
				break;
			case 1:
				EIMSK &= ~(1 << INT5);
				break;
			case 6:
				EIMSK &= ~(1 << INT6);
				break;
			case 7:
				EIMSK &= ~(1 << INT7);
				break;
#else
			case 0:
#if defined(EIMSK) && defined(INT0)
				EIMSK &= ~(1 << INT0);
#elif defined(GICR) && defined(ISC00)
				GICR &= ~(1 << INT0); // atmega32
#elif defined(GIMSK) && defined(INT0)
				GIMSK &= ~(1 << INT0);
#else
#error detachInterrupt not finished for this cpu
#endif
				break;

			case 1:
#if defined(EIMSK) && defined(INT1)
				EIMSK &= ~(1 << INT1);
#elif defined(GICR) && defined(INT1)
				GICR &= ~(1 << INT1); // atmega32
#elif defined(GIMSK) && defined(INT1)
				GIMSK &= ~(1 << INT1);
#else
#warning detachInterrupt may need some more work for this cpu (case 1)
#endif
				break;
#endif
			}

			intFunc[interruptNum] = 0;
		}
	}

	/*
void attachInterruptTwi(void (*userFunc)(void) ) {
twiIntFunc = userFunc;
}
	 */

#if defined(__AVR_ATmega32U4__)
	ISR(INT0_vect) {
		if(intFunc[EXTERNAL_INT_0])
			intFunc[EXTERNAL_INT_0]();
	}

	ISR(INT1_vect) {
		if(intFunc[EXTERNAL_INT_1])
			intFunc[EXTERNAL_INT_1]();
	}

	ISR(INT2_vect) {
		if(intFunc[EXTERNAL_INT_2])
			intFunc[EXTERNAL_INT_2]();
	}

	ISR(INT3_vect) {
		if(intFunc[EXTERNAL_INT_3])
			intFunc[EXTERNAL_INT_3]();
	}

	ISR(INT6_vect) {
		if(intFunc[EXTERNAL_INT_4])
			intFunc[EXTERNAL_INT_4]();
	}

#elif defined(EICRA) && defined(EICRB)

	ISR(INT0_vect) {
		if(intFunc[EXTERNAL_INT_2])
			intFunc[EXTERNAL_INT_2]();
	}

	ISR(INT1_vect) {
		if(intFunc[EXTERNAL_INT_3])
			intFunc[EXTERNAL_INT_3]();
	}

	ISR(INT2_vect) {
		if(intFunc[EXTERNAL_INT_4])
			intFunc[EXTERNAL_INT_4]();
	}

	ISR(INT3_vect) {
		if(intFunc[EXTERNAL_INT_5])
			intFunc[EXTERNAL_INT_5]();
	}

	ISR(INT4_vect) {
		if(intFunc[EXTERNAL_INT_0])
			intFunc[EXTERNAL_INT_0]();
	}

	ISR(INT5_vect) {
		if(intFunc[EXTERNAL_INT_1])
			intFunc[EXTERNAL_INT_1]();
	}

	ISR(INT6_vect) {
		if(intFunc[EXTERNAL_INT_6])
			intFunc[EXTERNAL_INT_6]();
	}

	ISR(INT7_vect) {
		if(intFunc[EXTERNAL_INT_7])
			intFunc[EXTERNAL_INT_7]();
	}

#else

	ISR(INT0_vect) {
		if(intFunc[EXTERNAL_INT_0])
			intFunc[EXTERNAL_INT_0]();
	}

	ISR(INT1_vect) {
		if(intFunc[EXTERNAL_INT_1])
			intFunc[EXTERNAL_INT_1]();
	}

#if defined(EICRA) && defined(ISC20)
	ISR(INT2_vect) {
		if(intFunc[EXTERNAL_INT_2])
			intFunc[EXTERNAL_INT_2]();
	}
#endif

#endif

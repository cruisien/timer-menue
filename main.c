/* Tiny TFT Graphics Library - see http://www.technoblogy.com/show?L6I

   David Johnson-Davies - www.technoblogy.com - 13th June 2019
   ATtiny85 @ 8 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#define F_CPU 8000000UL                 // set the CPU clock
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "st7735.h"


#define BACKLIGHT_ON PORTB |= (1<<PB2)
#define BACKLIGHT_OFF PORTB &= ~(1<<PB2)						

#define LED_OFF PORTC &= ~(1<<PC3)
#define LED_ON PORTC |= (1<<PC3)

//Buttons 


/* some RGB color definitions                                                 */
#define BLACK        0x0000      
#define RED          0x001F      
#define GREEN        0x07E0      
#define YELLOW       0x07FF      
#define BLUE         0xF800      
#define CYAN         0xFFE0      
#define White        0xFFFF     
#define BLUE_LIGHT   0xFD20      
#define TUERKISE     0xAFE5      
#define VIOLET       0xF81F		
#define WHITE		0xFFFF

#define SEK_POS 10,110

#define RELOAD_ENTPRELL 1 

// Pins already defined in st7735.c
extern int const DC;
extern int const MOSI;
extern int const SCK;
extern int const CS;
// Text scale and plot colours defined in st7735.c
extern int fore; 		// foreground colour
extern int back;      	// background colour
extern int scale;     	// Text size


volatile uint8_t ms10,ms100,sec,min, entprell;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
char stringbuffer[20]; // buffer to store string 
volatile uint8_t sek = 0;
volatile uint8_t ms = 0;
volatile uint8_t minu = 0;
volatile uint8_t std = 0;
volatile uint8_t clear = 0;
volatile uint8_t statestopp = 0;
volatile uint8_t count = 0; //0 = upp, 1 = down

#define HOMESCREEN 0
#define PAUSESTART 0
#define ZEIT 0
#define START 1
#define RESET 1
#define MENUE 2
#define STOPUHR 1
#define ZAEL 1
#define TIMER 2
#define TASTE_BLAU !(PIND & (1<<PD5))
#define TASTE_GELB !(PIND & (1<<PD6))
#define TASTE_ROT !(PIND & (1<<PD2))

ISR (TIMER1_COMPA_vect);
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ISR(TIMER0_OVF_vect)   //overflow Interrupt Service Routine von Timer0
{
	static uint8_t ISR_zaehler = 0;
	TCNT0 = 0; // Startwert des Zaehlers nach Interrupt
	ISR_zaehler++;

	if(ISR_zaehler == 12){ //entspricht ca 100ms sekundem (8M / 256 / 256 = 122/10)
		ms++;
		ISR_zaehler = 0;
		if(ms == 10){
			ms = 0;
			if(statestopp != PAUSESTART){
				if(count == 0){
				sek++;
			}
			else{sek--;}
			}//end pause
		}
		if(count == 0){
			if(sek == 60){
				sek = 0;
				clear = 1;
				minu++;
			}
			if(minu == 60){
				minu = 0;
				clear = 1;
				std++;
			}
		}//end countup
		else{
			if(sek > 60){
			sek = 60;
			clear = 1;
			minu--;	
			}
			if(minu > 60){
				minu = 60;
				clear = 1;
				std--;
			}
		}//end countdown
		
		
		
	} // end if z.5
	
}//End of ISR
void SELECTOR (void);
void SELECTORSTOP (void);
void BALL(void);
void BALLSTOP(void);
void BALLNEG(void);
void BALLNEGSTOP(void);
uint8_t SELECT (void);
uint8_t SELECTSTOP (void);

volatile uint8_t selector[2] = {86, 46};
volatile uint8_t selectorstop[3] = {58, 38, 18};
volatile uint8_t selectorpos = 0;// 0= stoppuhr, 1=timer
volatile uint8_t nselectorpos = 0;//ball reset
volatile uint8_t state = 0;
volatile uint8_t statetimer = 0;

int main(void)
{
	//----------------------------------------------------------------------------------------------------------------------
	
	char buffer [20];
	uint8_t statestop = 0;

	
	
	DDRB |= (1<<DC) | (1<<CS) | (1<<MOSI) |( 1<<SCK); 	// All outputs
	PORTB = (1<<SCK) | (1<<CS) | (1<<DC);          		// clk, dc, and cs high
	DDRB |= (1<<PB2);									//lcd Backlight output
	PORTB |= (1<<CS) | (1<<PB2);                  		// cs high
	DDRC |= (1<<PC3);									//Reset Output
	DDRD |= (1<<PD7);									//Reset Output
	PORTD |= (1<<PD7);	
									//Reset High
	DDRD &= ~((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//Taster 1-3
	PORTD |= ((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//PUllups für Taster einschalten
	
		//Timer 1 Configuration
	OCR1A = 1249;	//OCR1A = 0x3D08;==1sec
	
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    //Set interrupt on compare match

    TCCR1B |= (1 << CS11) | (1 << CS10);
    // set prescaler to 64 and start the timer

    sei();
    // enable interrupts
    
    ms10=0;
    ms100=0;
    sec=0;
    min=0;
    entprell=0;
	
	BACKLIGHT_ON;
	LED_ON;

	setup();
	//Konfiguration Timer Overflow
	TCCR0A = 0x00;        //normal mode
	TCCR0B     = 0x04;        //clk/256
	TIMSK0     |= (1 << TOIE0);// Timer0 frei
	TIFR0      |= (1 << TOV0);//Interrupt Timeroverflow.einschalten
	sei();                    //Globale interrupts freigeben
	
	//_-------------------------------------------------------------------------------------------------------------------------------
while(1){
	
	switch(state){
		case HOMESCREEN:	scale = 2;
							fore = CYAN;
							ClearDisplay();
							MoveTo(40, 80);
							PlotText(PSTR("Stoppuhr"));
							MoveTo(40, 40);
							PlotText(PSTR("Timer"));
							while(state == HOMESCREEN){
								static uint8_t tastenegB = 0;
								static uint8_t tastenegG = 0;
								static uint8_t tastenegR = 0;
								BALL();
								if(TASTE_BLAU > tastenegB){
									BALLNEG();
									selectorpos++;
									tastenegB = 1;//schalter einfach betätigen
									SELECTOR();
								}//end tasteblau
								if(TASTE_BLAU < tastenegB){
									tastenegB = 0;
									_delay_ms(200);
								}//end reset tastblau
								if(TASTE_GELB > tastenegG){
									BALLNEG();
									selectorpos--;
									tastenegG = 1;//schalter einfach betätigen
									SELECTOR();
								}//end tastegelb
								if(TASTE_GELB < tastenegG){
									tastenegG = 0;
									_delay_ms(200);
								}//end reset tastegelb
								if(TASTE_ROT > tastenegR){
									tastenegR = 1;
									state = SELECT();
								}//end tasterot
								if(TASTE_ROT < tastenegR){
									tastenegR = 0;
								}//end reset tasterot
							}//End while homescreen
							break;
		
		
		
		
		
		
		
		case STOPUHR:	ClearDisplay();
						sek = 0; minu = 0; std = 0;
						while(state != HOMESCREEN){
							switch(statestop){
								case PAUSESTART:
											if(clear == 1){
											ClearDisplay();
											clear = 0;
											}
											sprintf(buffer,"%d:%d:%d", std, minu, sek);
											MoveTo(10,100);
											fore=WHITE;
											PlotString(buffer);
											break;
											
								case RESET:	sek = 0; minu = 0; std = 0;
											sprintf(buffer,"%d:%d:%d ", std, minu, sek);
											MoveTo(10,100);
											fore=WHITE;
											PlotString(buffer);
											statestopp = PAUSESTART;
											break;
											
								case MENUE:	
											state = HOMESCREEN;
											statestop = PAUSESTART;
											break;
								}//end switch state timer
								fore = GREEN;
								MoveTo(40, 50);
								if(statestopp == PAUSESTART){
									PlotText(PSTR("START"));
									fore = WHITE;
									MoveTo(90, 101);
									FillRect(4, 16);
									MoveTo(98, 101);
									FillRect(4, 16);
									fore = GREEN;
								}
								else{
									PlotText(PSTR("PAUSE"));
									fore = BLACK;
									MoveTo(90, 101);
									FillRect(4, 16);
									MoveTo(98, 101);
									FillRect(4, 16);
									fore = GREEN;
								}//menue pause/start
							
							
							
							MoveTo(40, 30);
							PlotText(PSTR("RESET"));
							MoveTo(40, 10);
							PlotText(PSTR("MENUE"));
							static uint8_t tastenegBT = 0;
								static uint8_t tastenegGT = 0;
								static uint8_t tastenegRT = 0;
								BALLSTOP();
								if(TASTE_BLAU > tastenegBT){
									BALLNEGSTOP();
									selectorpos++;
									tastenegBT = 1;//schalter einfach betätigen
									SELECTORSTOP();
								}//end tasteblau
								if(TASTE_BLAU < tastenegBT){
									tastenegBT = 0;
									_delay_ms(200);
								}//end reset tastblau
								if(TASTE_GELB > tastenegGT){
									BALLNEGSTOP();
									selectorpos--;
									tastenegGT = 1;//schalter einfach betätigen
									SELECTORSTOP();
								}//end tastegelb
								if(TASTE_GELB < tastenegGT){
									tastenegGT = 0;
									_delay_ms(200);
								}//end reset tastegelb
								if(TASTE_ROT > tastenegRT){
									tastenegRT = 1;
									statestop = SELECTSTOP();
									if(statestop == 0){
										statestopp++;
										if(statestopp == 2){
											statestopp =0;
										}
									}//end pause unpause
								}//end tasterot
								if(TASTE_ROT < tastenegRT){
									tastenegRT = 0;
								}//end reset tasterot
							}//end while Stopuhr
							ClearDisplay();
							break;
						
						
						
		case TIMER: 		ClearDisplay();
							while(1){
								switch(statetimer){
									
									
									case MENUE:	
											state = HOMESCREEN;
											statestop = PAUSESTART;
											break;
								}//end switch timer
							fore = GREEN;
								MoveTo(40, 50);
								if(statestopp == PAUSESTART){
									PlotText(PSTR("START"));
									fore = WHITE;
									MoveTo(90, 101);
									FillRect(4, 16);
									MoveTo(98, 101);
									FillRect(4, 16);
									fore = GREEN;
								}
								else{
									PlotText(PSTR("PAUSE"));
									fore = BLACK;
									MoveTo(90, 101);
									FillRect(4, 16);
									MoveTo(98, 101);
									FillRect(4, 16);
									fore = GREEN;
								}//menue pause/start
							
							
							
							MoveTo(40, 30);
							PlotText(PSTR("ZEIT"));
							MoveTo(40, 10);
							PlotText(PSTR("MENUE"));
							static uint8_t tastenegBT = 0;
								static uint8_t tastenegGT = 0;
								static uint8_t tastenegRT = 0;
								BALLSTOP();
								if(TASTE_BLAU > tastenegBT){
									BALLNEGSTOP();
									selectorpos++;
									tastenegBT = 1;//schalter einfach betätigen
									SELECTORSTOP();
								}//end tasteblau
								if(TASTE_BLAU < tastenegBT){
									tastenegBT = 0;
									_delay_ms(200);
								}//end reset tastblau
								if(TASTE_GELB > tastenegGT){
									BALLNEGSTOP();
									selectorpos--;
									tastenegGT = 1;//schalter einfach betätigen
									SELECTORSTOP();
								}//end tastegelb
								if(TASTE_GELB < tastenegGT){
									tastenegGT = 0;
									_delay_ms(200);
								}//end reset tastegelb
								if(TASTE_ROT > tastenegRT){
									tastenegRT = 1;
									statestop = SELECTSTOP();
									if(statestop == 0){
										statestopp++;
										if(statestopp == 2){
											statestopp =0;
										}
									}//end pause unpause
								}//end tasterot
								if(TASTE_ROT < tastenegRT){
									tastenegRT = 0;
								}//end reset tasterot;
							}//end while timer
		
	}//end switch state
	
		
	}//End while z.153

	  
	 
}//end of main

ISR (TIMER1_COMPA_vect)
{
	
}

void SELECTOR (void){
	scale = 2;
	if(selectorpos == 2){
		selectorpos = 0;
	}//reset selectorpos to valid value
	if(selectorpos > 2){
		selectorpos = 1;
	}//reset selectorpos to valid value
	
	BALL();

	
}//end selector

void SELECTORSTOP (void){
	scale = 2;
	if(selectorpos == 3){
		selectorpos = 0;
	}//reset selectorpos to valid value
	if(selectorpos > 10){
		selectorpos = 2;
	}//reset selectorpos to valid value
	switch(selectorpos){
	}//end switch nselectorpos

	BALLSTOP();

	
}//end selectortimer

void BALL(void){
	//ball color
	uint8_t R = 4;
	fore = BLUE_LIGHT;
	//ball movement
	switch(R){
		case 10:glcd_draw_circle(15, selector[selectorpos], 10);
		case 9:glcd_draw_circle(15, selector[selectorpos], 9);
		case 8:glcd_draw_circle(15, selector[selectorpos], 8);
		case 7:glcd_draw_circle(15, selector[selectorpos], 7);
		case 6:glcd_draw_circle(15, selector[selectorpos], 6);
		case 5:glcd_draw_circle(15, selector[selectorpos], 5);
		case 4:glcd_draw_circle(15, selector[selectorpos], 4);
		case 3:glcd_draw_circle(15, selector[selectorpos], 3);
		case 2:glcd_draw_circle(15, selector[selectorpos], 2);
		case 1:glcd_draw_circle(15, selector[selectorpos], 1);
	
}
}

void BALLNEG(void){
	//ball color
	uint8_t R = 4;
	fore = BLACK;
	//ball movement
	switch(R){
		case 10:glcd_draw_circle(15, selector[selectorpos], 10);
		case 9:glcd_draw_circle(15, selector[selectorpos], 9);
		case 8:glcd_draw_circle(15, selector[selectorpos], 8);
		case 7:glcd_draw_circle(15, selector[selectorpos], 7);
		case 6:glcd_draw_circle(15, selector[selectorpos], 6);
		case 5:glcd_draw_circle(15, selector[selectorpos], 5);
		case 4:glcd_draw_circle(15, selector[selectorpos], 4);
		case 3:glcd_draw_circle(15, selector[selectorpos], 3);
		case 2:glcd_draw_circle(15, selector[selectorpos], 2);
		case 1:glcd_draw_circle(15, selector[selectorpos], 1);
	
}
}

void BALLSTOP(void){
	//ball color
	uint8_t R = 4;
	fore = BLUE_LIGHT;
	//ball movement
	switch(R){
		case 10:glcd_draw_circle(15, selectorstop[selectorpos], 10);
		case 9:glcd_draw_circle(15, selectorstop[selectorpos], 9);
		case 8:glcd_draw_circle(15, selectorstop[selectorpos], 8);
		case 7:glcd_draw_circle(15, selectorstop[selectorpos], 7);
		case 6:glcd_draw_circle(15, selectorstop[selectorpos], 6);
		case 5:glcd_draw_circle(15, selectorstop[selectorpos], 5);
		case 4:glcd_draw_circle(15, selectorstop[selectorpos], 4);
		case 3:glcd_draw_circle(15, selectorstop[selectorpos], 3);
		case 2:glcd_draw_circle(15, selectorstop[selectorpos], 2);
		case 1:glcd_draw_circle(15, selectorstop[selectorpos], 1);
		
	
}
}

void BALLNEGSTOP(void){
	//ball color
	uint8_t R = 4;
	fore = BLACK;
	//ball movement
	switch(R){
		case 10:glcd_draw_circle(15, selectorstop[selectorpos], 10);
		case 9:glcd_draw_circle(15, selectorstop[selectorpos], 9);
		case 8:glcd_draw_circle(15, selectorstop[selectorpos], 8);
		case 7:glcd_draw_circle(15, selectorstop[selectorpos], 7);
		case 6:glcd_draw_circle(15, selectorstop[selectorpos], 6);
		case 5:glcd_draw_circle(15, selectorstop[selectorpos], 5);
		case 4:glcd_draw_circle(15, selectorstop[selectorpos], 4);
		case 3:glcd_draw_circle(15, selectorstop[selectorpos], 3);
		case 2:glcd_draw_circle(15, selectorstop[selectorpos], 2);
		case 1:glcd_draw_circle(15, selectorstop[selectorpos], 1);
		
	
}
}

uint8_t SELECT (void){
	uint8_t stat = 0;
	switch(selectorpos){
		case 0:stat = STOPUHR; break;
		case 1:stat = TIMER; break;
	}//end switch select
	return(stat);
}//end SELECT

uint8_t SELECTSTOP (void){
	uint8_t statestop = 0;
	switch(selectorpos){
		case 0:statestop = PAUSESTART; break;
		case 1:statestop = RESET; break;
		case 2:statestop = MENUE; break;
	}//end switch selectstop
	return(statestop);
}//end selectstop


/*scale = 1;
	if(clear == 1){
		ClearDisplay();
		clear = 0;
	}

	sprintf(buffer,"%d:%d:%d", std, minu, sek);
	MoveTo(10,100);
	fore=WHITE;
	PlotString(buffer);
*/



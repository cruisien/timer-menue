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


char stringbuffer[20]; // buffer to store string 
volatile uint8_t sek = 50;
volatile uint8_t ms = 0;
volatile uint8_t minu = 59;
volatile uint8_t std = 0;
volatile uint8_t clear = 0;

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
			sek++;
		}
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
		
		
		
	} // end if z.5
	
}//End of ISR
int main(void)
{
	char buffer [20];
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
	
	
while(1){
	scale = 1;
	if(clear == 1){
		ClearDisplay();
		clear = 0;
	}

	sprintf(buffer,"%d:%d:%d", std, minu, sek);
	MoveTo(10,100);
	fore=WHITE;
	PlotString(buffer);
	
		
	}//End while z.20 

	  
	 
}//end of main

ISR (TIMER1_COMPA_vect)
{
	
}




/*#define F_CPU 8000000UL                 // set the CPU clock
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
/*#define BLACK        0x0000      
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


uint8_t sec = 0;
//ISR(TIMER0_OVF_vect)   //overflow Interrupt Service Routine von Timer0
{
	static uint8_t ISR_zaehler = 0;
	TCNT0 = 0; // Startwert des Zaehlers nach Interrupt
	ISR_zaehler++;
	if(ISR_zaehler == 12){ //entspricht ca 100ms sekundem (8M / 256 / 256 = 122/10)
		ISR_zaehler = 0;
		sec++;
		
	} // end if z.5
	
}//End of ISR


int main(void){
	char buffer [20];
	BACKLIGHT_ON;
	LED_ON;

	setup();
	
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
	
	//Konfiguration Timer Overflow
	TCCR0A = 0x00;        //normal mode
	TCCR0B     = 0x04;        //clk/256
	TIMSK0     |= (1 <<TOIE0);//Interrupt Timeroverflow.einschalten
	sei();                    //Globale interrupts freigeben
	
	while(1){
		fore=WHITE;
	MoveTo(10,10);
	PlotText(buffer);
	sprintf(buffer, " %d",sec);
		
	}//End while z.20 
	
}//end main void z.12          //Globale interrupts freigeben*/

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

#define TASTE_BLAU !(PIND & (1<<PD5))
#define TASTE_GELB !(PIND & (1<<PD6))
#define TASTE_ROT !(PIND & (1<<PD2))

ISR (TIMER1_COMPA_vect);
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
volatile uint8_t sek;


ISR(TIMER0_OVF_vect)   //overflow Interrupt Service Routine von Timer0
{
	static uint8_t ISR_zaehler = 0;
	TCNT0 = 0; // Startwert des Zaehlers nach Interrupt
	ISR_zaehler++;

	if(ISR_zaehler == 3){ //entspricht ca 100ms sekundem (8M / 256 / 256 = 122/10)
		
		ISR_zaehler = 0;
		
			
				sek++;
			}
			
							
	
}//End of ISR

void SPI_MasterTransmit(uint8_t cData)
{
/* Start transmission */
SPDR = cData;
/* Wait for transmission complete */
while(!(SPSR & (1<<SPIF)))
;
}

int main(void)
{
	//----------------------------------------------------------------------------------------------------------------------
	
	char buffer [20];

		//init spi
	SPCR |= (1<<SPE) | (1<<MSTR);
	SPSR |= (1<<SPI2X);
	
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
		if(TASTE_ROT == 1){
			sek = 0;
			MoveTo(0, 0);
			fore = RED;
			FillRect(42,42);
			MoveTo(0,42);
			fore = YELLOW;
			FillRect(42,42);
			MoveTo(0,84);
			fore = BLUE;
			FillRect(42,44);
			MoveTo(42,0);
			fore = BLUE_LIGHT;
			FillRect (42,42);
			MoveTo(42,42);
			fore = GREEN;
			FillRect (42,42);
			MoveTo (42, 84);
			fore = VIOLET;
			FillRect(42,44);
			MoveTo (84,0);
			fore = CYAN;
			FillRect (44,42);
			MoveTo (84, 42);
			fore = TUERKISE;
			FillRect (44, 42);
			MoveTo (84,84);
			fore = RED;
			FillRect (44, 44);
			
			sprintf (buffer, "%d", sek);
			MoveTo (42,42);
			fore = BLUE;
			scale = 2;
			
			PlotString(buffer);
			PlotText(PSTR("*25 = MS"));
		}//end if taste rot
		
		
	}//end main while

	  
	 
}//end of main

ISR (TIMER1_COMPA_vect)
{
	
}




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



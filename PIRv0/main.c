/*
 * PIRv0.c
 *
 * Created: 28.03.2018 19:40:25
 * Author : Ami_go
 */ 

// ATMEL ATTINY24 / ARDUINO
//                                           Actual PINOUT
//                                               +-\/-+
//                                         VCC  1|    |14  GND
//[*position 1] (1min )            (D  0)  PB0  2|    |13  AREF (D 10)   (60 min)    [*position 4]
//[*position 2] (5min)             (D  1)  PB1  3|    |12  PA1  (D  9)   (SPARE)
//             *Reset              (D 11)  PB3  4|    |11  PA2  (D  8)   Comparator input
//             (pirout) PWM  INT0  (D  2)  PB2  5|    |10  PA3  (D  7)   (triac)
//[*position 3](30 min) PWM        (D  3)  PA7  6|    |9   PA4  (D  6)            *SCK
//             *MOSI    PWM        (D  4)  PA6  7|    |8   PA5  (D  5)        PWM *MISO

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL  // 1 MHz
#include <util/delay.h>


//macros definition
#define SET_BIT(PORT_NR,BIT) (PORT_NR |= (1<<BIT))
#define CLEAR_BIT(PORT_NR,BIT) (PORT_NR &= ~(1<<BIT))
#define bit_is_clear(sfr,bit) (!(_SFR_BYTE(sfr) & _BV(bit)))


//Switch soldering definition
#define SW0B   PIN0  /*POSITION 1 - 1 minutes;  SW0 <-> SWCOM */
#define SW1B   PIN1  /*POSITION 2 - 5 minutes;  SW1 <-> SWCOM */
#define SW2A   PIN7  /*POSITION 3 - 30 minutes; SW2 <-> SWCOM */
#define SW3A   PIN0  /*POSITION 4 - 60 minutes; SW3 <-> SWCOM */
#define SW4A   PIN1 /*NOT USED in current project*/
#define light  PINA3
#define comp_in PINA2   

#define one_minute 3000  // one tick is equal of 20 ms 

//************************************************
// Global variable structure
//************************************************

typedef struct
{
   char light_enable;
   unsigned int one_min_counter;
   unsigned int min_counter;
   
}global_var;

//************************************************
// Global variable declaration
//************************************************
volatile global_var gvar;


//functions prototypes 

//void analogCompInt(void);
char switch_position(void);
void GPIOInt(void);
void SleepInt(void);
void Sleep_on(void);
void Sleep_off(void);
void PCIntIRQInt(void);
//Interrupt service request part 

/*ISR(ANA_COMP_vect){
	Sleep_off();
	if (gvar.light_enable == 1) {
		
		
		
	   if(bit_is_clear(PINA, comp_in)){
			_delay_us(200);
			SET_BIT(PINA,light);
			_delay_us(700);
			CLEAR_BIT(PINA, light);
		}
		
		else {
			
			SET_BIT(PINA,light);
			_delay_us(700);
			CLEAR_BIT(PINA, light);
		} 
	Sleep_on();	
	}
	
}*/

ISR(PCINT0_vect){
		Sleep_off();
		if (gvar.light_enable == 1) {
			
			if(bit_is_clear(PINA, comp_in)){
				_delay_us(300);
				SET_BIT(PORTA,light);
				_delay_us(700);
				CLEAR_BIT(PORTA, light);
			}
			
			else {
				
				SET_BIT(PORTA,light);
				_delay_us(700);
				CLEAR_BIT(PORTA, light);
			}
			Sleep_on();
		}
}
ISR(PCINT1_vect){
	
}



//************************************************
// MAIN SOFTWARE PART
//************************************************

int main(void)
{
    GPIOInt(); 
//    analogCompInt();
    PCIntIRQInt();
	sei();
	SleepInt();
	
	
	gvar.light_enable = 1;
	Sleep_on();
    /* Replace with your application code */
    while (1) 
    {
	 gvar.light_enable = 1;
   }
}

void GPIOInt (void){
	
	SET_BIT(DDRA,light);  //triac pin as output
	
	//switch pins configured as input. 	
	CLEAR_BIT(DDRB, SW0B);
	CLEAR_BIT(DDRB, SW1B);
	CLEAR_BIT(DDRA, SW2A);
	CLEAR_BIT(DDRA, SW3A);
}

void SleepInt(void){
	//disable burnout detector sequence 
	SET_BIT(MCUCR,BODS); 
	SET_BIT(MCUCR,BODSE); 
	_delay_us(1);
	SET_BIT(MCUCR,BODS); 
	CLEAR_BIT(MCUCR,BODSE); 
	_delay_us(5);
	
	//SET IDLE sleep mode
//	CLEAR_BIT(MCUCR,SM0);
//	CLEAR_BIT(MCUCR,SM1);
	
	//set power down mode
	  CLEAR_BIT(MCUCR,SM0);
	  SET_BIT(MCUCR,SM1);
}
void Sleep_on (void){
	SET_BIT(MCUCR,SE); 
}
void Sleep_off(void){
	CLEAR_BIT(MCUCR,SE); 
}

char switch_position(void ){
	
	char sw_position = 0;
	
	
	//Turn on internal pull-up resistors
	SET_BIT(PORTA,SW2A);
	SET_BIT(PORTA,SW3A);
	SET_BIT(PORTB,SW0B);
	SET_BIT(PORTB,SW1B); 
		
	_delay_us(500);
	
	//Reading switch
	if(bit_is_clear(PINB, SW0B))  sw_position = 0;
	if(bit_is_clear(PINB, SW1B))  sw_position = 1;
	if(bit_is_clear(PINA, SW2A))  sw_position = 2;
	if(bit_is_clear(PINA, SW3A))  sw_position = 3;
	
	//Turn on internal pull-up resistors
	CLEAR_BIT(PORTA,SW2A);
	CLEAR_BIT(PORTA,SW3A);
	CLEAR_BIT(PORTB,SW0B);
	CLEAR_BIT(PORTB,SW1B);

	return sw_position;
	
}
void PCIntIRQInt(void){
	
	CLEAR_BIT(MCUCR,ISC01); //any logical changes 
	SET_BIT(MCUCR,ISC00);
	
	SET_BIT(PCMSK0,PCINT2); // comparator pin
	
//	SET_BIT(PCMSK1,PCINT10); //  pir pin
	
	SET_BIT(GIMSK,PCIE0); // comparator pin
	SET_BIT(GIMSK,PCIE1); // pir pin
	 
}
/*void analogCompInt(void){
	//initialize the analog comparator (AC)
	//DIDR0 &= ~((1<<ADC2D) | (1<<ADC1D)); //disable digital buffer on pins AIN0 && AIN1 to reduce current consumption
			
	SET_BIT(DIDR0,ADC0D);
	SET_BIT(DIDR0,ADC1D);
	
	CLEAR_BIT(ACSR,ACIE);//disable interrupts on AC
	CLEAR_BIT(ACSR,ACD); //switch on analog comparator
	SET_BIT(ACSR,ACBG);  //set Internal Voltage Reference (1V1)
	CLEAR_BIT(ACSR,ACO); // disable output
	CLEAR_BIT(ACSR,ACIC); // disable input capture mode
	CLEAR_BIT(ACSR,ACIS0); //interrupt on toggle event
	CLEAR_BIT(ACSR,ACIS1);
	
	SET_BIT(ACSR,ACIE);   //enable interrupts on AC
	} */

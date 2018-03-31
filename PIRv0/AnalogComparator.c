#include "AnalogComparator.h"


void analogCompInt(void){
	
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
	
}
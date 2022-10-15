// ***** 1. Pre-processor Directives Section *****
#include "tm4c123gh6pm.h"
#include "SysTick.h"

 // Port E Def
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_CR_R         (*((volatile unsigned long *)0x40024524))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
// Port A Def
#define GPIO_PORTA_DATA_R       (*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_CR_R         (*((volatile unsigned long *)0x40004524))
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
// Port F Def
#define GPIO_PORTF_DATA_BITS_R  ((volatile unsigned long *)0x40025000)
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_IS_R         (*((volatile unsigned long *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile unsigned long *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile unsigned long *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile unsigned long *)0x40025410))
#define GPIO_PORTF_RIS_R        (*((volatile unsigned long *)0x40025414))
#define GPIO_PORTF_MIS_R        (*((volatile unsigned long *)0x40025418))
#define GPIO_PORTF_ICR_R        (*((volatile unsigned long *)0x4002541C))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_DR2R_R       (*((volatile unsigned long *)0x40025500))
#define GPIO_PORTF_DR4R_R       (*((volatile unsigned long *)0x40025504))
#define GPIO_PORTF_DR8R_R       (*((volatile unsigned long *)0x40025508))
#define GPIO_PORTF_ODR_R        (*((volatile unsigned long *)0x4002550C))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_PDR_R        (*((volatile unsigned long *)0x40025514))
#define GPIO_PORTF_SLR_R        (*((volatile unsigned long *)0x40025518))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define GPIO_PORTF_ADCCTL_R     (*((volatile unsigned long *)0x40025530))
#define GPIO_PORTF_DMACTL_R     (*((volatile unsigned long *)0x40025534))
//clock
#define SYSCTL_RCGC2_R 					(*((volatile unsigned long *)0x400FE108))
	
// ***** 2. Global Declarations Section *****

struct State {
	unsigned long Output;
	unsigned long Turn;
	unsigned long Time;    // 10 ms units
	unsigned long Next[8]; // list of next states
};
typedef const struct State STyp;

#define goW 0			// Output is 0x0C
#define waitW 1		// Output is 0x14
#define goS 2			// Output is 0x21
#define waitS 3		// Output is 0x22
#define goRT 4		// Output is 0x21
#define waitRT 5	// Output is 0x22

STyp FSM[6] = {
{0x0C, 0x02, 11,{goW,waitW,goW,waitW,waitW,waitW,waitW,waitW}}, 	      	  // gOW
{0x14, 0x02, 8,{goS,goRT,goS,goRT,goS,goRT,goS,goRT}}, 									  // waitW
{0x21, 0x02, 11,{goS,goRT,waitS,goRT,goS,goRT,waitS,goRT}}, 			  // goS
{0x22, 0x02, 8,{goW,goW,goW,goW,goW,goW,goW,goW}}, 								  	// waitS
{0x21, 0x08, 11,{goRT,goRT,waitRT,waitRT,goRT,goRT,waitRT,waitRT}}, 					  // goRT
{0x22, 0x0A, 8,{goW,goW,goW,goW,goW,goW,goW,goW}}			// WaitRT
};


// FUNCTION PROTOTYPES: Each subroutine defined
#define SENSOR 							  (*((volatile unsigned long *)0x40004070)) //access A0 - A2 Sensors
#define VEHICLE_LIGHTS       (*((volatile unsigned long *)0x400240FC))	//accesses PE5(RedE)-PE4(YellowE)-PE3(GreenE)-PE2(RedS)-PE1(YellowS)–PE0(GreenS)
#define TURNING_LIGHTS   		 (*((volatile unsigned long *)0x40025028))	//accesses PF3 and PF1	
unsigned long CS;
unsigned long Input;
void portA_init(void);
void portE_init(void);
void portF_init(void);
void SysTick_Wait500ms(unsigned long delay);

// ***** 3. Subroutines Section *****

int main(void){ 
	portA_init();
    portE_init();
	portF_init(); 
	SysTick_Init();
	
    CS = goW;             // Initializing State0

     while(1){ 
    		// Assigning variables to FSM values
       VEHICLE_LIGHTS = FSM[CS].Output;  
    		TURNING_LIGHTS = FSM[CS].Turn;
    		// Delay
    		SysTick_Wait500ms(FSM[CS].Time);
    		// Taking Input
    		Input = SENSOR >> 2 ; // read sensors
    		// Define Next State
    		CS = FSM[CS].Next[Input];
    	}
}



void portA_init(void){volatile unsigned long delay;
	SYSCTL_RCGC2_R = 0x00000001; //Port A Clock
  delay = SYSCTL_RCGC2_R; // delay           
  GPIO_PORTA_CR_R = 0x1C; // allow changes to PA3 and PA2 (buttons)              
  GPIO_PORTA_AMSEL_R = 0x00; // disables analog functions
  GPIO_PORTA_PCTL_R = 0x00000000; //GPIO clear bit PCTL
  GPIO_PORTA_DIR_R = 0x00; // set direction to input for PA3 and PA2
  GPIO_PORTA_AFSEL_R = 0x00; // no alternate functions
  GPIO_PORTA_DEN_R = 0x1C;   // enable digital pins PA3 and PA2
}

void portE_init(void){volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x0000010;   //Port E Clock 
  delay = SYSCTL_RCGC2_R;        // delay       
  GPIO_PORTE_CR_R = 0x3F;        // allow changes to ALL P0-P5 (LEDs)         
  GPIO_PORTE_AMSEL_R = 0x00;     // disables analog functions   
  GPIO_PORTE_PCTL_R = 0x00000000;//GPIO clear bit PCTL    
  GPIO_PORTE_DIR_R = 0x3F;       // set direction to ouput ALL P0-P5     
  GPIO_PORTE_AFSEL_R = 0x00;     // no alternate functions   
  GPIO_PORTE_DEN_R = 0x3F;       // enable digital pins ALL P0-P5
}

void portF_init(void){volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     //  F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0A;          // PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // enable digital pins PF4-PF0   
}
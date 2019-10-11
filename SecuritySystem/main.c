/* Author: Aaron Grogg, Nate Weber
 * Date: 5/1/2019
 * Honor code: " We have neither given or received,
 * nor have I tolerated others' us of unauthorized aid"
 * Purpose: ECE422 - Final Project
 * Description: Driver for a two-zone office security system
 */

#include <msp430.h> 
#include "LCD.h"

#define SMCLK   0x0200  // Timer SMCLK source
#define UP      0x0010  // Timer UP count
#define ACLK    0x0100  // Timer ACLK source

//PROTOTYPES
void short_buzz(int hertz);
void mode_select(void);
void sound_alarm();

//GLOBAL VARIABLES
unsigned long Code = 0;
int codeFlag = 0;
int Alarm = 0;                              //Alarm starts off!
unsigned long MODE1 = 111111;
unsigned long MODE2 = 222222;
unsigned long MODE3 = 333333;
unsigned long MODE4 = 444444;
int ActiveMode = 0;
int ZoneAlarm = 0;


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	            // Stop Watchdog Timer
	PM5CTL0 = 0xFFFE;                       // Enable Pins Globally
    LCD_init();                             // LCD initialization function

    TA1CCR0=37200;                          // ~1s with ACLK
    TA1CCTL0 = 0x0010;                      // Enable interrupt for TA0

    //** BUZZER SETUP **//
    P2DIR |= BIT0;                          // Buzzer will use P2.0

    //** ZONE LEDS **//
    P9DIR |= BIT5;                          // P9.5 as output
    P9DIR |= BIT6;                          // P9.6 as output
    P9OUT &= ~BIT5;                         // Start off
    P9OUT &= ~BIT6;                         // Start off

	//** KEYPAD SETUP **//
	P1IES |= BIT3 + BIT6 + BIT7;            // Interrupt edge select
	P1IE  |= BIT3 + BIT6 + BIT7;            // Enable interrupt
	P1REN |= BIT3 + BIT6 + BIT7;            // Resistor enable
	P1OUT |= BIT3 + BIT6 + BIT7;            // Pull up
	P1IFG = 0;                              // Clear Port1 IFG

	P4DIR |= BIT0 + BIT1 + BIT2 + BIT3;     // Configure P3.0 - P3.3 as output
	P4OUT |= BIT0 + BIT1 + BIT2 + BIT3;
	P4OUT &= ~(BIT0 + BIT1 + BIT2 +BIT3);

	display_word("Enter a code!");

	_BIS_SR( GIE );                         // Enable global interrupts

	while(1){
	    if(codeFlag){                       // If a code was entered
	        codeFlag = 0;
	        mode_select();
	    }
	    if(Alarm){
	        sound_alarm();
	    }
	}
	//Alarm system will remain on forever
}

//**************************************************
// Interrupt service routine for keypad button press
//**************************************************
#pragma vector=PORT1_VECTOR
__interrupt void port_1(void) {
    P4DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3);
    if(ActiveMode == 4){
        TA1CTL = ACLK + UP + TACLR;     //reset clock
    }else{
        TA1CTL = 0x0104;            // Stop timer
    }
    static int i = 0;
    static long num=0;
    switch(P1IV){                   // Switch on Port 1 IV
        case(0x08):                 // Last column pressed
            if(P4IN &= 0x01){       // If # is pressed
                if(!Alarm){
                    short_buzz(1000);
                    med_delay();
                    short_buzz(1000);
                    med_delay();
                }
                codeFlag = 1;
                num = 0;
            }else if(P4IN &= 0x02){ // If 9 is pressed
                num=num%1000000;        // Math to calculate 9
                num*=10;
                num+=9;
                if(!Alarm){
                    short_buzz(600);
                }
            }else if(P4IN &= 0x04){ // If 6 is pressed
                num=num%1000000;        // Math to calculate 6
                num*=10;
                num+=6;
                if(!Alarm){
                    short_buzz(550);
                }
            }else{
                num=num%1000000;        // If 3 is pressed
                num*=10;            // Math to calculate 3
                num+=3;
                if(!Alarm){
                    short_buzz(500);
                }
            }
            break;
        case(0x0E):                 // Middle column
            if(P4IN &= 0x01){       // If 0 is pressed
                num=num%1000000;        // Math to calculate 0
                num*=10;
                if(!Alarm){
                    short_buzz(500);
                }
            }else if(P4IN &= 0x02){ // If 8 is pressed
                num=num%1000000;        // Math to calculate 8
                num*=10;
                num+=8;
                if(!Alarm){
                    short_buzz(600);
                }
            }else if(P4IN &= 0x04){ // If 5 is pressed
                num=num%1000000;        // Math to calculate 5
                num*=10;
                num+=5;
                if(!Alarm){
                    short_buzz(550);
                }
            }else{                  // If 2 is pressed
                num=num%1000000;        // Math to calculate 2
                num*=10;
                num+=2;
                if(!Alarm){
                    short_buzz(500);
                }
            }
            break;
        case(0x10):                 // First column
            if(P4IN &= 0x01){       // If * is pressed
                num-=num%10;        // Math to delete a number
                num/=10;
                if(!Alarm){
                    short_buzz(450);
                    med_delay();
                    short_buzz(450);
                }
            }else if(P4IN &= 0x02){ // If 7 is pressed
                num=num%1000000;        // Math to calculate 7
                num*=10;
                num+=7;
                if(!Alarm){
                    short_buzz(600);
                }
            }else if(P4IN &= 0x04){ // If 4 is pressed
                num=num%1000000;        // Math to calculate 4
                num*=10;
                num+=4;
                if(!Alarm){
                    short_buzz(550);
                }
            }else{                      // If 1 is pressed
                num=num%1000000;        // Math to calculate 1
                num*=10;
                num+=1;
                if(!Alarm){
                    short_buzz(500);
                }
            }
        break;
    }
    P4DIR |= BIT0 + BIT1 + BIT2 + BIT3;     // Configure P4.0 - P4.3 as output
    P1IFG = 0;                              // Clear Port1 IFG

    if(i==0){
        i++;
        num = 0;
    }
    else if(codeFlag==0){
        if(num > 999999){
            clear_display();
            display_word("Max length...");
            short_buzz(300);
            med_delay();
            short_buzz(300);
            med_delay();
            clear_display();
            display_word("Code - ");
            num-=num%10;                // Math to delete a number
            num/=10;
            display_number(num);
        }else{
            Code = num;
            clear_display();
            display_word("Code - ");
            display_number(num);
        }
    }
}

//**************************************************
// Buzzer function to sound a short time (P2.0)
//**************************************************
void short_buzz(int hertz){
    int i;
    TA3CCR0 = hertz;                 //Set control register
    TA3CTL = TACLR | UP | SMCLK;    //Start counting
    for(i = 0; i < 25; i++){
        while((TA3CTL & TAIFG) == 0);   //Wait for flag
        if (i%2 == 0) {
            P2OUT |= BIT0;                  //Set buzzer on
        }else{
            P2OUT &= ~BIT0;                 //Set buzzer off
        }
        TA3CTL &= ~TAIFG;               //Clear flag
    }
    TA3CTL = TA3CTL & (~0x30);      //Stop timer
}

//**************************************************
// Buzzer function to sound the alarm
//**************************************************
void sound_alarm() {
    int i=0;
    int hertz=500;
    TA2CCR0 = hertz;                    //Set control register
    TA2CTL = TACLR | UP | SMCLK;        //Start counting
    while(Alarm){
        if(hertz == 600){               //Reset hertz back down to 500
            hertz = 500;
        }
        while((TA2CTL & TAIFG) == 0);   //Wait for flag

        if(ZoneAlarm == 1){
            if(i%50 == 0){
                P9OUT ^= BIT5;
            }
        }
        if(ZoneAlarm == 2){
            if(i%50 == 0){
                P9OUT ^= BIT6;
            }
        }
        if (i%3 == 0) {
            P2OUT |= BIT0;              //Set buzzer on
        }else{
            P2OUT &= ~BIT0;             //Set buzzer off
        }
        i++;
        hertz+=5;                       //Add 5 hertz to alarm
        TA2CTL &= ~TAIFG;               //Clear flag
        TA2CCR0 = hertz;                //Set control register
        if(codeFlag){
            codeFlag = 0;
            mode_select();
        }
    }
    ZoneAlarm = 0;
    P9OUT &= ~BIT5 + ~BIT6;
    TA2CTL = TA2CTL & (~0x30);          //Stop timer
}

void set_mode1(void){
    //Both Zones Armed
    P9OUT |= BIT6;
    P9OUT |= BIT5;
    P3IFG = 0;                              // Clear Port3 IFG
    P3IE  |= BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7;
    display_word("                Current Mode: 1");
    cursor_home();
}

void set_mode2(void){
    //Only Zone 2 Armed
    P9OUT |= BIT6;
    P9OUT &= ~BIT5;
    P3IFG = 0;                              // Clear Port3 IFG
    P3IE  &= ~BIT0 + ~BIT1 + ~BIT2;
    P3IE  |= BIT3 + BIT6 + BIT7;
    display_word("                Current Mode: 2");
    cursor_home();
}
void set_mode3(void){
    //All Disarmed
    P9OUT &= ~BIT6;
    P9OUT &= ~BIT5;
    P3IFG = 0;                              // Clear Port3 IFG
    P3IE  &= ~BIT0 + ~BIT1 + ~BIT2 + ~BIT3 + ~BIT6 + ~BIT7;
    display_word("                Current Mode: 3");
    cursor_home();
}

void set_mode4(void){
    TA1CTL = ACLK + UP + TACLR;     //reset clock
    Code = 0;
    clear_display();
    display_word("Enter mode!");
    while(Code == (!MODE1|!MODE2|!MODE3)){
        while(!codeFlag);
        switch(Code){
        case 1: Code = 0;
                codeFlag = 0;
                clear_display();
                display_word("Changing Mode 1");
                long_delay();
                clear_display();
                display_word("Enter new code!");
                while(!codeFlag);
                if((Code == MODE1)|(Code == MODE2)|(Code == MODE3)|(Code == MODE4)){
                    clear_display();
                    display_word("Invalid entry!");
                    long_delay();
                }else{
                    clear_display();
                    display_word("Success!");
                    long_delay();
                    MODE1 = Code;
                }
                codeFlag = 0;
                break;
        case 2: Code = 0;
                codeFlag = 0;
                clear_display();
                display_word("Changing Mode 2");
                long_delay();
                clear_display();
                display_word("Enter new code!");
                while(!codeFlag);
                if((Code == MODE1)|(Code == MODE2)|(Code == MODE3)|(Code == MODE4)){
                    clear_display();
                    display_word("Invalid entry!");
                    long_delay();
                }else{
                    clear_display();
                    display_word("Success!");
                    long_delay();
                    MODE2 = Code;
                }
                codeFlag = 0;
                break;
        case 3: Code = 0;
                codeFlag = 0;
                clear_display();
                display_word("Changing Mode 3");
                long_delay();
                clear_display();
                display_word("Enter new code!");
                while(!codeFlag);
                if((Code == MODE1)|(Code == MODE2)|(Code == MODE3)|(Code == MODE4)){
                    clear_display();
                    display_word("Invalid entry!");
                    long_delay();
                }else{
                    clear_display();
                    display_word("Success!");
                    long_delay();
                    MODE3 = Code;
                }
                codeFlag = 0;
                break;
        case 4: Code = 0;
                codeFlag = 0;
                clear_display();
                display_word("Changing Mode 4");
                long_delay();
                clear_display();
                display_word("Enter new code!");
                while(!codeFlag);
                if((Code == MODE1)|(Code == MODE2)|(Code == MODE3)|(Code == MODE4)){
                    clear_display();
                    display_word("Invalid entry!");
                    long_delay();
                }else{
                    clear_display();
                    display_word("Success!");
                    long_delay();
                    MODE4 = Code;
                }
                codeFlag = 0;
                break;
        }
    }
    TA1CTL = 0x0104;            // Stop timer
    ActiveMode = 3;
    set_mode3();
    display_word("Enter a code!");
}

void mode_select(void){
    if (Code == MODE1){
        ActiveMode = 1;
        clear_display();
        display_word("1&2 - Enabled");
        long_delay();
        //enable interrupts for all sensors (both zones)
        clear_display();
        set_mode1();    //sets mode 1
        display_word("Enter a code!");
    }else if(Code == MODE2){
        ActiveMode = 2;
        clear_display();
        display_word("2 - Enabled");
        long_delay();
        clear_display();
        set_mode2();    //sets mode 2
        display_word("Enter a code!");
    }else if(Code == MODE3){
        Alarm = 0;              //Make sure alarm is not sounding
        ActiveMode = 3;
        clear_display();
        display_word("Alarm Disabled");
        long_delay();
        clear_display();
        set_mode3();    //Turn off all sensors
        display_word("Enter a code!");
    }else if(Code == MODE4){
        Alarm = 0;              //Make sure alarm is not sounding
        ActiveMode = 4;
        clear_display();
        display_word("Admin Mode");
        long_delay();
        //Disable alarms
        set_mode4();    //admin mode
    }else if(Code == 911){
        Alarm = 1;      //Sound the alarm!
    }else{
        clear_display();
        display_word("Incorrect code!");//Incorrect code
        med_delay();
        med_delay();
        clear_display();
        display_word("Enter a code!");
        Code = 0;
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_ISR (void) {
    static int time=0;
    time++;
    if(time>=15){
        Code = MODE3;
        codeFlag = 1;
        time = 0;
    }
}

#pragma vector=PORT3_VECTOR
__interrupt void port_3(void) {
    Alarm = 1;                          //Turn alarm on!

    switch(P3IFG){                  // Switch on Port 3 IV
        //Zone 1
        case(BIT0):                 // Zone 1 Door (P3.0)
                ZoneAlarm = 1;
                break;
        case(BIT1):                 // Zone 1 Motion (P3.1)
                ZoneAlarm = 1;
                break;
        case(BIT2):                 // Zone 1 Window (P3.2)
                ZoneAlarm = 1;
                break;
        //Zone 2
        case(BIT3):                 // Zone 2 Door (P3.3)
                ZoneAlarm = 2;
                break;
        case(BIT6):                 // Zone 2 Motion (P3.6)
                ZoneAlarm = 2;
                break;
        case(BIT7):                 // Zone 2 Window (P3.7)
                ZoneAlarm = 2;
                break;
    }
    P3IFG = 0;                      // Clear Port3 IFG
}



/* Pin Layout
        Pin 2.4 RS
        Pin 2.5 E
        Pin 2.2 DB4
        Pin 2.1 DB5
        Pin 2.6 DB6
        Pin 2.7 DB7
*/

//Include Section
#include "LCD.h"
#include "string.h"
#include "msp430.h"
#include <string.h>

//Define variables for ease of use
#define RS_HIGH         P2OUT = P2OUT | BIT4    // define RS high
#define RS_LOW          P2OUT = P2OUT & (~BIT4) // define RS low
#define ENABLE_HIGH     P2OUT = P2OUT | BIT5    // define Enable high signal
#define ENABLE_LOW      P2OUT = P2OUT & (~BIT5) // define Enable Low signal
#define SMCLK   0x0200
#define UP      0x0010


// ***********************************************************
// Main initialization function for LCD screen
// ***********************************************************
void LCD_init(void){
    P2DIR |= BIT1 + BIT2 + BIT4 + BIT5 + BIT6 + BIT7;  // Configure outputs

    delay(15);                                  //Delay 40ms
    send_command(0,0,1,1);                      //Send initialization command
    delay(5);                                   //Delay 5ms
    send_command(0,0,1,1);                      //Send initialization command
    delay(1);                                   //Delay 1ms
    send_command(0,0,1,1);                      //Send initialization command
    send_command(0,0,1,0);                      //Set LCD to 4 bit mode

    send_command(0,0,1,0);                      // Turn two lines and font on
    send_command(1,1,0,0);

    send_command(0,0,0,0);                      // Turn LCD/Cursor/Blink on
    send_command(1,1,1,1);

    send_command(0,0,0,0);                      // Clear LCD screen
    send_command(0,0,0,1);

    send_command(0,0,0,0);                      // Turn increment on
    send_command(0,1,1,0);

    clear_display();                            // Clear LCD screen
}

// ***********************************************************
// Write function to toggle enable to force LCD write
// ***********************************************************
void write(void) {
    ENABLE_HIGH;            //Set enable high
    delay(5);               //Delay for 5ms
    ENABLE_LOW;             //Set enable low
}


// ***********************************************************
// Clear display function to clear LCD Screen
// ***********************************************************
void clear_display() {
    delay(2);
    send_command(0,0,0,0);  //Send commands for clear
    send_command(0,0,0,1);
    delay(2);
}

// ***********************************************************
// Show character function to display a single character
// ***********************************************************
void show_char(char c) {
    int U1,U2,U3,U4,L1,L2,L3,L4;

    //Grab upper 4 bits
    U4 = checkBit(c & (1 << (7 - 0)));
    U3 = checkBit(c & (1 << (7 - 1)));
    U2 = checkBit(c & (1 << (7 - 2)));
    U1 = checkBit(c & (1 << (7 - 3)));

    //Grab lower 4 bits
    L4 = checkBit(c & (1 << (7 - 4)));
    L3 = checkBit(c & (1 << (7 - 5)));
    L2 = checkBit(c & (1 << (7 - 6)));
    L1 = checkBit(c & (1 << (7 - 7)));

    delay(2);                           //Delay 2ms
    send_data(U4,U3,U2,U1);             //Send upper 4 bits
    send_data(L4,L3,L2,L1);             //Send lower 4 bits
    delay(2);                           //Delay 2ms
}

// ***********************************************************
// Display word function to display a string on LCD
// ***********************************************************
void display_word(char word[]) {
    unsigned int i;
    for(i = 0; i < strlen(word); i++){          //Display every char
        if(i < 16){                             //First line
            show_char(word[i]);                 //Show character
        } else {                                //Second line
            set_cursor_position(0x40 + i - 16); //Set cursor to next line
            show_char(word[i]);                 //Show character
        }
    }
}

// ***********************************************************
// Set cursor position on LCD screen
// ***********************************************************
void set_cursor_position(int position){
    //Grab upper/lower 4 bits
    char upperBits=position>>4;
    char lowerBits=position<<4;
    lowerBits=lowerBits>>4;
    delay(5);                       //Delay 5ms

    //Pass upper four bits
    char DB7 = 1;
    char DB6 = checkBit(upperBits & 0b0100);
    char DB5 = checkBit(upperBits & 0b0010);
    char DB4 = checkBit(upperBits & 0b0001);
    send_command(DB7,DB6,DB5,DB4);
    delay(1);

    //Pass lower four bits
    DB7 = checkBit(lowerBits & 0b1000);
    DB6 = checkBit(lowerBits & 0b0100);
    DB5 = checkBit(lowerBits & 0b0010);
    DB4 = checkBit(lowerBits & 0b0001);
    send_command(DB7,DB6,DB5,DB4);
    delay(1);
}

// ***********************************************************
// Returns cursor to "home" or first position
// ***********************************************************
void cursor_home() {            //Set cursor to home position
    P2OUT &= ~BIT4;             //Instruction register selected
    set_cursor_position(0);     //Set position to first location
}


// ***********************************************************
// Blinks a string for set number of times
// ***********************************************************
void very_slow_blink(char a[], int numBlinks){
    clear_display();                //Clear LCD display
    int tmpCount = 0;
    int i=0;
    for(i=0; i < numBlinks; i++){   //Blink x times
        for(tmpCount=0; tmpCount<100; tmpCount++){  //Longer delay
            delay(10);
        }
        display_word(a);            //Display word
        for(tmpCount=0; tmpCount<100; tmpCount++){  //Longer delay
            delay(10);
        }
        clear_display();            //Clear display
    }

}

// ***********************************************************
// Cursor shift R=1 (right) R=0 (left)
// ***********************************************************
void cursor_shift(int right){
    if(right==1) {              //shift right
        delay(1);               //delay 1ms
        send_command(0,0,0,1);  //send command
        delay(1);               //delay 1ms
        send_command(1,1,1,1);  //send command
    } else {                    //shift left
        delay(1);               //delay 1ms
        send_command(0,0,0,1);  //send command
        delay(1);               //delay 1ms
        send_command(1,0,1,1);  //send command
    }
}

// ***********************************************************
// Scroll words across LCD screen
// ***********************************************************
void scroll_words(char a[]){
    unsigned int i;
    static int position = 0x10;
    set_cursor_position(position);      //Set cursor position to top right
    for(i=0; i < strlen(a); i++){   //Continue for every character
        if(position == 0x28){
            position = 0x00;
            set_cursor_position(position);
        }
        show_char(a[i]);            //Show character
        cursor_shift(0);            //Shift cursor to left
        delay(60);                  //Delay
        delay(60);
        delay(60);
        delay(60);
        delay(60);
        position++;
    }
    for(i=0; i<16; i++){           //Shift 16 times
        show_char(' ');            //Show character
        cursor_shift(0);           //"clear" screen
        delay(60);
        delay(60);
        delay(60);
        delay(60);
        delay(60);
    }
    clear_display();
}

// ***********************************************************
// Displays a number on LCD (0-999)
// ***********************************************************
void display_number(long num){
    if(num > 99999) {              //if num is three digits
        int x,y,z,w,a,b;
        b = num % 10;           //grab last digit
        num = num / 10;
        a = num % 10;           //grab last digit
        num = num / 10;
        w = num % 10;           //grab last digit
        num = num / 10;
        z = num % 10;           //grab last digit
        num = num / 10;
        y = num % 10;           //grab middle digit
        num = num / 10;
        x = num;                //grab first digit
        show_char(x + 48);      //show numbers
        show_char(y + 48);
        show_char(z + 48);
        show_char(w + 48);
        show_char(a + 48);
        show_char(b + 48);
    } else if(num > 9999) {              //if num is three digits
        int x,y,z,w,a;
        a = num % 10;           //grab last digit
        num = num / 10;
        w = num % 10;           //grab last digit
        num = num / 10;
        z = num % 10;           //grab last digit
        num = num / 10;
        y = num % 10;           //grab middle digit
        num = num / 10;
        x = num;                //grab first digit
        show_char(x + 48);      //show numbers
        show_char(y + 48);
        show_char(z + 48);
        show_char(w + 48);
        show_char(a + 48);
    } else if(num > 999) {              //if num is three digits
        int x,y,z,w;
        w = num % 10;           //grab last digit
        num = num / 10;
        z = num % 10;           //grab last digit
        num = num / 10;
        y = num % 10;           //grab middle digit
        num = num / 10;
        x = num;                //grab first digit
        show_char(x + 48);      //show numbers
        show_char(y + 48);
        show_char(z + 48);
        show_char(w + 48);
    } else if(num > 99) {              //if num is three digits
        int x,y,z;
        z = num % 10;           //grab last digit
        num = num / 10;
        y = num % 10;           //grab middle digit
        num = num / 10;
        x = num;                //grab first digit
        show_char(x + 48);      //show numbers
        show_char(y + 48);
        show_char(z + 48);
    } else if(num > 9) {        //if number is two digits
        int x,y;
        y = num % 10;           //grab last digit
        num = num / 10;
        x = num;                //grab first digit
        show_char(x + 48);      //show numbers
        show_char(y + 48);
    } else {                    //if num is one digit
        show_char(num + 48);    //show number
    }
}

// ***********************************************************
// Send data function to send characters and data
// ***********************************************************
void send_data(int a, int b, int c, int d) {
    RS_HIGH;                //RS set high to send data
    if(a == 0){             //Sets first register to 1 or 0
        P2OUT &= ~BIT7;
    } else {
        P2OUT |= BIT7;
    }
    if(b == 0){             //Sets second register to 1 or 0
        P2OUT &= ~BIT6;
    } else {
        P2OUT |= BIT6;
    }
    if(c == 0){             //Sets third register to 1 or 0
        P2OUT &= ~BIT1;
    } else {
        P2OUT |= BIT1;
    }
    if(d == 0){             //Sets fourth register to 1 or 0
        P2OUT &= ~BIT2;
    } else {
        P2OUT |= BIT2;
    }
    delay(2);               //Delay
    write();                //Toggle enable to force write
}


// ***********************************************************
// Send command function to send instructions and settings
// ***********************************************************
void send_command(int a, int b, int c, int d){
    RS_LOW;                 //RS set low to pass instructions
    switch(a){              //Sets first register to 1 or 0
        case 0:
            P2OUT &= ~BIT7;
            break;
        case 1:
            P2OUT |= BIT7;
            break;
    }
    switch(b){              //Sets second register to 1 or 0
        case 0:
            P2OUT &= ~BIT6;
            break;
        case 1:
            P2OUT |= BIT6;
            break;
    }
    switch(c){              //Sets third register to 1 or 0
        case 0:
            P2OUT &= ~BIT1;
            break;
        case 1:
            P2OUT |= BIT1;
            break;
    }
    switch(d){              //Sets fourth register to 1 or 0
        case 0:
            P2OUT &= ~BIT2;
            break;
        case 1:
            P2OUT |= BIT2;
            break;
    }
    delay(2);               //Delay 2ms
    write();                //Toggle enable to force write
}

// ***********************************************************
// Delay function to wait X ms
// ***********************************************************
void delay(long long int time) {
    time *= 1000;                   //Adjust time to ms
    TA0CCR0 = time;                 //Set control register
    TA0CTL = TACLR | UP | SMCLK;    //Start counting
    while((TA0CTL & TAIFG) == 0);   //Wait for flag
    TA0CTL &= ~TAIFG;               //Clear flag
    TA0CTL = TA0CTL & (~0x30);      //Stop timer
}

// ***********************************************************
// Delay function to wait for extra long time
// ***********************************************************
void long_delay() {
    int i;
    for(i = 0; i < 200; i++){   //Loop 250 times
        delay(10);              //Delay 25ms
    }
}

// ***********************************************************
// Delay function to wait for med time
// ***********************************************************
void med_delay() {
    int i;
    for(i = 0; i < 35; i++){   //Loop 250 times
        delay(10);              //Delay 25ms
    }
}

// ***********************************************************
// Check bit 1/0 by value in bit
// ***********************************************************
int checkBit(int x){
    if(x >= 1){     //if x is greater than 1
        return 1;   //return 1
    }else{          //else
        return 0;   //return 0
    }
}

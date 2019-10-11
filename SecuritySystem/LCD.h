
#ifndef LCD_H_
#define LCD_H_

//***** Prototypes ************************************************************
void clear_display( void );
void display_word( char[] );
int  checkBit( int );
void show_char( char );
void med_delay(void);
void delay(long long int);
void long_delay(void);
void write(void);
void LCD_init(void);
void send_command(int, int, int, int);
void send_data(int, int, int, int);
void cursor_home(void);
void set_cursor_position(int);
void cursor_shift(int);
void scroll_words(char[]);
void display_number(long);
void very_slow_blink(char[], int);

#endif /* LCD_H_ */

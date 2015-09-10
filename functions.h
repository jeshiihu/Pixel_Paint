#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// standard U of A library settings, assuming Atmel Mega SPI pins
#define SD_CS    5  // Chip select line for SD card
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

// constructor that implements the changes to the lcd screen
extern Adafruit_ST7735 tft;
//extern Sd2Card card;

#define WHITE ST7735_WHITE
#define BLACK ST7735_BLACK
#define RED ST7735_RED
#define YELLOW ST7735_YELLOW
#define GREEN ST7735_GREEN
#define CYAN ST7735_CYAN
#define BLUE ST7735_BLUE
#define MAGENTA ST7735_MAGENTA

// Constants for LCD screen
extern const int WIDTH;
extern const int HEIGHT;

// Constants for the Joystick and cursor
extern const int JOYSTICK_VERT;   // Analog input A0 - vertical
extern const int JOYSTICK_HORIZ;   // Analog input A1 - horizontal
extern const int JOYSTICK_BUTTON;  // Digital input pin 9 for the button

// constants for the potentiometer and size leds
extern const int SIZE_DIAL; // Analog input A2
extern const int SIZE_LED[3]; // LED outputs


extern int initial_joystick_y, initial_joystick_x; // for joystick calibration

// initializes global variables 
extern char mode; // Mode: (p)encil (e)raser
extern char current_shape; // shape mode: (r)ectangle, (c)ircle, (t)riangle
extern int cursor_border;  // colours may be black, red, blue, white
extern int cursor_size; // size of the cursor drawn
extern int current_colour;

extern int cursor_y; // cursor at center of lcd
extern int cursor_x; 
extern int prev_cursor_y; // declares variable to be used in loop
extern int prev_cursor_x;
extern int size;
extern int x_bound;
extern int y_bound;

extern int pencil_colour; // declares variables to store previous pencil mode
extern char pencil_shape; // when user returns from eraser mode

extern uint8_t all_pixels[32][136];
extern int start; // to make sure icons are drawn at start
// ensures that the cursor and style icon updates immediately 
extern int icon_click; 


// forward declarations of functions
void draw_background();
void initialize_colour_array();
void eraser();
void clear();
void change_colour();
void draw_cursor(int, int, int, char, int);
void change_shape();
int size_selection(int);
void point_led(int);
void bits_to_colour(int, int, int);
void bits_to_colour(int, int, int, int);
void store_colour(int, int, int, int);
void bounds();
void pencil();
void save_pixel(int, int, int);

#endif

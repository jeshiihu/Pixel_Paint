// Project: Pixel Paint
// Jessica Huynh and Hailey Musselman

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include "functions.h"


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Constants for LCD screen
const int WIDTH = 128;
const int HEIGHT = 160;

// Constants for the Joystick and cursor
const int JOYSTICK_VERT = 0;   // Analog input A0 - vertical
const int JOYSTICK_HORIZ = 1;   // Analog input A1 - horizontal
const int JOYSTICK_BUTTON = 9;  // Digital input pin 9 for the button

// constants for the potentiometer and size leds
const int SIZE_DIAL = 2; // Analog input A2
const int SIZE_LED[3] = {2,3,4}; // LED outputs

int cursor_y; // current y position of cursor (topmost pixels of cursor)
int cursor_x; // current x position of cursor (leftmost pixels of cursor)
int prev_cursor_y = 0; // declares variable to be used in loop
int prev_cursor_x = 0;

// grabs resting position of joystick for calibration
int initial_joystick_y = analogRead(JOYSTICK_VERT); // range from 0-1023
int initial_joystick_x = analogRead(JOYSTICK_HORIZ); // range from 0-1023

int cursor_size = 8;
int current_colour = BLUE;
char current_shape = 'r';
char mode = 'p';
int cursor_border = 0;
int pencil_colour = BLUE;
char pencil_shape = 'r';
int start = 1;
int icon_click = 0;

Sd2Card card;

/**
   2D array containing colours of every pixel in the drawing region

   - each element of array contains data from 4 pixels...
   - there are 4 possible colours :
   WHITE = 0b00, BLACK = 0b01, RED = 0b10, BLUE = 0b11
   - So, each colour can be represented by 2 bits
   - 8 bits/byte, 2 bits/pixel => 4 pixels/byte => 4 pixels/uint8_t
   (since uint8_t is 1 byte)

   - width of region = 128, height of region = 136
   - (128 pixels wide)/(4 pixels/uint8_t) = 32 uint8_t elements to
   contain all pixel data
   in a horizontal row
   - 136 uint32_t elements to index all horizontal rows
*/
uint8_t all_pixels[32][136];

void setup() {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB); // initialize a ST7735R chip, red tab
  pinMode(JOYSTICK_BUTTON, INPUT);
  digitalWrite(JOYSTICK_BUTTON, HIGH);

  // configure LED Pins to be the outputs
  for (int i = 0; i < 3; ++i) {
    pinMode(SIZE_LED[i], OUTPUT);
    digitalWrite(SIZE_LED[i], HIGH);
  }

  // Checking to see if SD is initialized and can be read
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    return;
  }
  Serial.println("OK!");
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("Raw SD Initialization has failed");
    while (1) {};  // Just wait, stuff exploded.
  }

  // clear screen to white
  tft.fillScreen(ST7735_WHITE);

  // grabs resting position of joystick for calibration
  initial_joystick_y = analogRead(JOYSTICK_VERT); // range from 0-1023
  initial_joystick_x = analogRead(JOYSTICK_HORIZ); // range from 0-1023

  initialize_colour_array();
  draw_background();

  cursor_x = WIDTH/2 - cursor_size/2;
  cursor_y = 68 - cursor_size/2;
  draw_cursor(cursor_x, cursor_y, cursor_size, current_shape, current_colour);
    
  start = 0;
}

void loop() {
  // reads the postion of the joystick and its values
  int joystick_y = analogRead(JOYSTICK_VERT);
  int joystick_x = analogRead(JOYSTICK_HORIZ);

  // to change the size of the cursor
  int point = analogRead(SIZE_DIAL);
  int size = map(point, 0, 1023,4,12);
  point_led(size);

  // if size changes, redraw
  if (size_selection(size)) {
    bits_to_colour(cursor_x,cursor_y, 12);
    draw_background();
    cursor_border = 1;
    draw_cursor(cursor_x,cursor_y,cursor_size,current_shape,current_colour);
  }

  // +10, -10 is to calibrate the joystick by not causing a cursor drift
  // while the joystick is not being used

  // when the joytick is pushed upwards, cursor moves up one
  if(joystick_y > (initial_joystick_y+10)) {
    --cursor_y;

    // when joystick it pushed downwards, cursor moves down one
  } else if(joystick_y < (initial_joystick_y-10)) {
    ++cursor_y;
  }

  // when the joytick is pushed rightwards, cursor moves left one
  if(joystick_x > (initial_joystick_x+10)) {
    --cursor_x;

    // when joystick is pushed leftwards, cursor moves right one
  } else if(joystick_x < (initial_joystick_x-10)) {
    ++cursor_x;
  }

  // prevents cursor form moving past the bounds of the lcd screen
  bounds();

  // when the joystick is not pressed down, pencil acts as a cursor
  // only make changes if the cursor has moved (except when clicking in icons)
  if(digitalRead(JOYSTICK_BUTTON) == HIGH) {
    if ((cursor_x != prev_cursor_x) || (cursor_y != prev_cursor_y) 
	|| icon_click == 1) {
	
      // Erase the previous cursor by redrawing over the cursor with
      // the background colour
      if (prev_cursor_y < 136) { // in drawing region
	icon_click = 0;
	// redraw what was "underneath" cursor
	bits_to_colour(prev_cursor_x, prev_cursor_y, cursor_size);
      }
      // creates new cursor postion from  movement of joystick
      cursor_border = 1; // cursor only requires border when not drawing
      draw_cursor(cursor_x,cursor_y,cursor_size,current_shape,current_colour);
      icon_click = 0;
      // delay so that cursor does not move too quickly
      // bigger cursor needs less delay because it takes more
      // time to redraw
      if (cursor_size == 4) {
	delay(30);
      } else if (cursor_size == 8) {
	delay(20); 
      } else {
	delay(10);
      }  
    }
  }

  // when the joystick is pressed down pencil acts as a drawing tool
  if (digitalRead(JOYSTICK_BUTTON) == LOW) {
    if (cursor_y < 136) {
      icon_click = 0;
      draw_cursor(cursor_x, cursor_y, 
		  cursor_size, current_shape,
		  current_colour);
      store_colour(cursor_x, cursor_y, cursor_size, current_colour);
      delay(30); // delay so that cursor does not move too quickly

    } else {
      icon_click = 1;
      // prevents user from holding down and moving the joystick
      // as there is no use for that in the icon region
      while(digitalRead(JOYSTICK_BUTTON) == LOW) {}
      if(cursor_x < 24) { // selecting colour for pencil
	if(mode == 'p') { // when in eraser mode you cannot change colour
	  change_colour();
	}
      } else if(cursor_x > 24 && cursor_x < 51) { // selecting pencil mode
	pencil();
      } else if(cursor_x > 51 && cursor_x < 76) { // selecting eraser mode
	eraser();
      } else if(cursor_x >76 && cursor_x < 102) { // changes shape of cursor
	change_shape();
      } else if(cursor_x > 102) { // selecting function to clear to white
	clear();
      }
    }
  }

  // updates the previous cursor position to current cursor
  prev_cursor_y = cursor_y;
  prev_cursor_x = cursor_x;
}


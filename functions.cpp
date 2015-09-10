#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include "functions.h"


// FUNCTION: draws and displays the icons at the bottom
// RUNTIME: O(1)
void draw_background() {

  // top horizontal line
  tft.drawLine(0, 136, WIDTH-1, 136, BLACK);

  // 4 vertical dividers
  tft.drawLine(24, 137, 24, HEIGHT-1, BLACK);
  tft.drawLine(50, 137, 50, HEIGHT-1, BLACK);
  tft.drawLine(76, 137, 76, HEIGHT-1, BLACK);
  tft.drawLine(102, 137, 102, HEIGHT-1, BLACK);

  if(cursor_x <= 24 || start == 1) {
    // colour palatte - 1st from left
    tft.fillRect(0, 137, 12, 12, BLACK);
    tft.fillRect(12, 137, 12, 12, RED);
    tft.fillRect(0, 148, 12, 12, WHITE);
    tft.fillRect(12, 148, 12, 12, BLUE);
  }

  if((cursor_x > 24-cursor_size && cursor_x <=50) || start == 1) {
    // pencil - 2nd from left
    tft.fillRect(25, 137, 25, 23, WHITE);
    tft.fillRoundRect(33, 152, 9, 6, 2, MAGENTA); // eraser
    tft.fillRect(33, 145, 9, 9, 0xFBE0); // body 
    tft.fillTriangle(33, 144, 41, 144, 37, 138, YELLOW); // pointed end
    tft.drawPixel(37, 139, BLACK); // tip
    tft.fillRect(36, 140, 3, 1, BLACK); // tip
  }

  if((cursor_x > 50-cursor_size && cursor_x <= 76) || start == 1) {
    // eraser - 3rd from left
    tft.fillRect(51, 137, 25, 23, WHITE);
    tft.fillRoundRect(59, 140, 10, 11, 2, MAGENTA);
    tft.fillRoundRect(59, 151, 10, 6, 2, BLUE);
    tft.fillRect(59, 150, 10, 2, BLUE);
  }

  // cursor style - 4th from left
  tft.fillRect(77, 137, 25, 23, WHITE);
  
  if(current_shape == 'r') { // square
    tft.fillRect(85, 145, 8, 8, current_colour);
    tft.drawRect(85, 145, 8, 8, BLACK);
  } else if(current_shape == 'c') { // circle
    tft.fillCircle(89,149,4,current_colour);
    tft.drawCircle(89, 149, 4, BLACK);
  } else if(current_shape == 's') { // slash
    tft.drawLine(84,152,93,143, current_colour);
    tft.drawLine(84,153,94,143, current_colour);
    tft.drawLine(85,153,94,144, current_colour);
    
    tft.drawPixel(84,153, BLACK);
    tft.drawLine(84,152,93,143, BLACK);
    tft.drawLine(85,153,94,144,BLACK);
    tft.drawPixel(94,143, BLACK);
  }

  if((cursor_x > 102-cursor_size) || start == 1) {
    // clear icon ('X') - 5th from left
    tft.fillRect(103, 137, 25, 23, WHITE);
    tft.drawLine(103, 137, WIDTH-1, HEIGHT-1, RED);
    tft.drawLine(103, HEIGHT-1, WIDTH-1, 137, RED);
  }
}

// FUNCTION: changes pencil colour to colour selected
// RUNTIME: O(1)
void change_colour() {
  int half_cursor = cursor_size/2;
  int x = cursor_x+half_cursor;
  int y = cursor_y+half_cursor;

  // each colour is a different region
  if(x < 12 && y < 148) {
    current_colour = BLACK;
  } else if(x < 12 && y > 148) {
    current_colour = WHITE;
  } else if(x > 12 && y < 148) {
    current_colour = RED;
  } else if(x > 12 && y > 148) {
    current_colour = BLUE;
  }
}

// FUNCTION: saves previous state of the pencil and turns cursor to an eraser
// RUNTIME: O(1)
void eraser() {
  mode = 'e';
  pencil_colour = current_colour;
  pencil_shape = current_shape;
  current_colour = WHITE;
}

// FUNCTION: reverts to colour and shape of pencil before eraser mode
// RUNTIME: O(1)
void pencil() {
  mode = 'p';
  current_colour = pencil_colour; 
  current_shape = pencil_shape;
}

// FUNCTION: clears the drawing space
// RUNTIME: O(1)
void clear() {
  // paint white rectangle over drawing surface
  tft.fillRect(0, 0, WIDTH-1, 136, WHITE);
  initialize_colour_array();
}

// FUNCTION: changes the shape when user clicks on the changing shape icon
// RETURNS: current shape (r,c,s)
// RUNTIME: O(1) (time increased slightly if current shape is not (r)ectangle)
void change_shape() {
  draw_background();
  char shape_array[] = "rcs"; // the 3 possible shapes

  if(current_shape == 's') {
    current_shape = 'r';
  } else { // changes shape to the next in the array (r->c->s) and loops
    for(int i=0; i<3; ++i) {
      if(current_shape == shape_array[i]) {
	current_shape = shape_array[i+1];
        break;
      }
    }
  }
}

// FUNCTION: draws the the cursor with respect to the 
// specified colour, shape, and size
// RUNTIME: O(1)
void draw_cursor(int x,int y,int size, char shape, int colour) {
  int half_size = size/2;

  if(shape == 'r') { // for a square
    tft.fillRect(x, y, size, size, colour);
    if(cursor_border == 1){ // black border
      tft.drawRect(x, y, size, size, GREEN);
      cursor_border = 0;
    }
  } else if(shape == 'c') { // for a circle
    tft.fillCircle(x + half_size, y + half_size, half_size, colour);
    if(cursor_border == 1){ // black border
      tft.drawCircle(x + half_size, y + half_size, half_size, GREEN);
      cursor_border = 0;
    }
  } else if(shape == 's') { // for a slash
    tft.drawLine(x, y + size - 1, x + size - 1, y, colour);
    tft.drawLine(x, y + size - 2, x + size - 2, y, colour);
    tft.drawLine(x + 1, y + size - 1, x + size - 1, y + 1, colour);
    if(cursor_border == 1){ // black border
      tft.drawLine(x, y + size - 2, x + size - 2, y, GREEN);
      tft.drawLine(x + 1, y + size - 1, x + size - 1, y + 1, GREEN);
      tft.drawPixel(x, y + size - 1, GREEN);
      tft.drawPixel(x + size - 1, y, GREEN);
      cursor_border = 0;
    }
  }
}

// FUNCTION: translates LED status to cursor size (4, 8, 12)
// RETURNS: 1 if size has changed; 0 if not
// RUNTIME: O(n) - tests 3 LEDs in a for loop
int size_selection(int size) {
  int prev_size = cursor_size;
  if(size == 4 || size ==8 || size == 12) {
    for (int i = 2; i >= 0; --i) {
      if (digitalRead(SIZE_LED[i]) == HIGH) { // checks which LEDs are on
	cursor_size = (4*i)+size;
      }
    }
  }
  if (cursor_size != prev_size) return 1;
  else return 0;
}

// FUNCTION: Turns on respective LEDS for each size
// RUNTIME: O(1)
void point_led(int size) {
  if(size == 4) { // 1 LED for small
    digitalWrite(SIZE_LED[0], HIGH);
    digitalWrite(SIZE_LED[1], LOW);
    digitalWrite(SIZE_LED[2], LOW);
  } else if(size == 8) { // 2 LEDs for medium
    digitalWrite(SIZE_LED[0], HIGH);
    digitalWrite(SIZE_LED[1], HIGH);
    digitalWrite(SIZE_LED[2], LOW);
  } else if(size == 12) { // 3 LEDs for large
    digitalWrite(SIZE_LED[0], HIGH);
    digitalWrite(SIZE_LED[1], HIGH);
    digitalWrite(SIZE_LED[2], HIGH);
  } else {}
}

// FUNCTION: keeps the cursor within the lcd screen
// RUNTIME: O(1)
void bounds() {

  // the circle is 1 pixel wider and 1 pixel taller
  // than the other 2 cursors
  int size;
  if(current_shape == 'r' || current_shape == 's') {
    size = cursor_size;
  } else { 
    size = cursor_size + 1;
  }
    
  if(cursor_y < 0) { // top boundary of map
    ++cursor_y;
  }
  if(cursor_y > HEIGHT - size) { // bottom boundary of map
    --cursor_y;
  }
  if(cursor_x < 0) { // left boundary of map
    ++cursor_x;
  }
  if(cursor_x > WIDTH - size) { // right boundary of map
    --cursor_x;
  }

  // if in icons region, redraw to account for cursor movement
  if((cursor_y > 136 - (size + 1)) &&
     ((cursor_x != prev_cursor_x) || (cursor_y != prev_cursor_y) || 
      icon_click == 1)) {
    draw_background();
  }
}

// FUNCTION: records all pixels in drawing space as white
// RUNTIME: O(n^2) - iterates through all rows AND columns
void initialize_colour_array() {
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 136; ++j) {
      all_pixels[i][j] = 0;
    }
  }
}

// FUNCTION: redraws the pixels in the region through which the cursor
// passes so that cursor movement does not change the drawing
// unintentionally
// RUNTIME: O(n^2) 
//     - one for loop
//     - two twice-nested for loops 
//     - while loop simply multiplies by the constant 1, 2, or 3
//     => twice-nested for loop has largest growth rate and so 
//        determines running time
void bits_to_colour(int prev_cursor_x, int prev_cursor_y, int cursor_size) {
    
  int temp_cursor_x = prev_cursor_x;
  int two_bits;
  int x_offset = temp_cursor_x % 4;
    
  // to redraw for cursors of different sizes
  int cursor_tracker = cursor_size/4;
    
  while (cursor_tracker > 0) {
    /*
      The binary numbers 0b11000000, 0b00110000, 0b00001100, 0b00000011
      are used to "select" from one uint8_t element in the pixels array
      the two bits of interest (i.e. the colour of a single pixel of
      interest).  These numbers correspond, in the order above, to decimal
      192, 48, 12, 3, which are all 3*(a multiple of 4).  The loop below
      generates the necessary value for a particular horizontal coordinate
      of a pixel.
    */
    int bits = 3;
    for (int i = 0; i < (4 - x_offset) - 1; ++i) {
      bits *= 4;
    }
        
    /*
      We look at groups of 4 horizontally adjacent pixels because this
      works well with the size of a uint8, and makes changing the cursor
      size by multiples of 4 convenient.
    */
        
    // redraw the colours in the 1st columm of uint8s used to store the
    // 4 pixels
    for (int i = temp_cursor_x;
	 i < temp_cursor_x + 4 - x_offset;
	 ++i, bits /= 4) {
      for (int j = prev_cursor_y; 
	   j < prev_cursor_y + cursor_size + 1;
	   ++j) {
	if (j > 135) break; // don't redraw over icons
	two_bits = bits & all_pixels[temp_cursor_x/4][j];
	bits_to_colour(two_bits, bits, i, j);
      }
    }
        
    // redraw the colours in the 2nd column of uint8s used to store the
    // 4 pixels
    for (int i = temp_cursor_x + 4 - x_offset, bits = 192;
	 i < temp_cursor_x + 4 + 1; // "+1" for extra width of circle
	 ++i, bits /= 4) {
      for (int j = prev_cursor_y; 
	   j < prev_cursor_y + cursor_size + 1;  
	   ++j) {
	if(j > 135) break; // don't redraw over icons
	two_bits = bits & all_pixels[temp_cursor_x/4 + 1][j];
	bits_to_colour(two_bits, bits, i, j);
      }
    }
        
    --cursor_tracker;
    temp_cursor_x += 4;
  }   
}

// FUNCTION: redraws a single pixel
// RUNTIME: O(1)
void bits_to_colour(int two_bits, int bits, int x, int y) {

  // compare 2 bits of interest to reference bits
  // to determine the colour to draw
  if (two_bits == 0) {
    tft.drawPixel(x, y, WHITE);
  } else if (two_bits == bits/3) {
    tft.drawPixel(x, y, BLACK);
  } else if (two_bits == bits*2/3) {
    tft.drawPixel(x, y, RED);
  } else if (two_bits == bits) {
    tft.drawPixel(x, y, BLUE);
  }
}

// FUNCTION: stores the colour drawn by the cursor in a 
// particular size and shape in the array of all pixels
// RUNTIME: O(n^2)
//     - many loops, with different ones executed based on
//       shape condition
//     - twice-nested for loop has highest growth rate
//       and so determines running time
void store_colour(int cursor_x, int cursor_y, 
                  int cursor_size, int current_colour) {

  if (current_shape == 'r') { // square of pixels
    for (int i = cursor_x; i < cursor_x + cursor_size; ++i) {
      for (int j = cursor_y; j < cursor_y + cursor_size; ++j) {
	save_pixel(i, j, current_colour);
      }
            
    }

  } else if (current_shape == 'c') { // circle of pixels
    // the shape of the circle is dependent on its size,
    // so there are 3 cases     
    if (cursor_size == 4) { // small cursor

      // save the rows of length cursor_size + 1
      for (int i = cursor_x; i < cursor_x + cursor_size + 1; ++i) {
	for (int j = cursor_y + 1; 
	     j < cursor_y + cursor_size; 
	     ++j) {
	  save_pixel(i, j, current_colour);
	}
      }

      // save the rows of length cursor_size - 1
      for (int i = cursor_x + 1; i < cursor_x + cursor_size; ++i) {
	save_pixel(i, cursor_y, current_colour);
	save_pixel(i, cursor_y + cursor_size, current_colour);
      }    
        
    } else if (cursor_size == 8) { // medium cursor

      // save the rows of length cursor_size + 1
      for (int i = cursor_x; i < cursor_x + cursor_size + 1; ++i) {
	for (int j = cursor_y + 3; 
	     j < cursor_y + 6; 
	     ++j) {
	  save_pixel(i, j, current_colour);
	}
      }      

      // save the rows of length cursor_size - 1
      for (int i = cursor_x + 1; i < cursor_x + cursor_size; ++i) {
	for (int j = cursor_y + 1; 
	     j < cursor_y + 3; 
	     ++j) {
	  save_pixel(i, j, current_colour);
	  save_pixel(i, j + 5, current_colour);
	}
      }

      // save the rows of length cursor_size - 3
      for (int i = cursor_x + 3; i < cursor_x + 6; ++i) { 
	save_pixel(i, cursor_y, current_colour);
	save_pixel(i, cursor_y + cursor_size, current_colour);
      }

    } else if (cursor_size == 12) { // large cursor
            
      // save the rows of length cursor_size + 1
      for (int i = cursor_x; i < cursor_x + cursor_size + 1; ++i) {
	for (int j = cursor_y + 4; 
	     j < cursor_y + 9; 
	     ++j) {
	  save_pixel(i, j, current_colour);
	}
      }      
            
      // save all other rows, which have lengths in increments
      // of 2 between 5 and 11
      int x_start = cursor_x + 4;
      int x_end = cursor_x + 8;
      int j = cursor_y;
      int y_index_incr = 12;            
      for (; x_start > cursor_x; 
	   --x_start, ++x_end, ++j, y_index_incr -= 2) {
	for (int i = x_start; i <= x_end; ++i) {
	  save_pixel(i, j, current_colour);
	  save_pixel(i, j + y_index_incr, current_colour);
	}
      }
    }
        
  } else if (current_shape == 's') { 
    // line of pixels from top right to bottom left of
    // cursor_size X cursor_size "box" 

    int i, j;
    for (i = cursor_x + cursor_size - 1, j = cursor_y; 
	 i >= cursor_x; --i, ++j) { 
      save_pixel(i, j, current_colour);            
      if (i == cursor_x) {
	// don't add extra pixels on the end of the line
	break;
      }          
      save_pixel(i - 1, j, current_colour);
      save_pixel(i, j + 1, current_colour);
    }       
  }
} 

// FUNCTION: saves the colour of a single pixel
// RUNTIME: O(1)
void save_pixel(int x, int y, int colour) {

  // position of the 2 bits in the uint8_t that 
  // store the pixel
  int hori_offset = x % 4;  
   
  // x_index in the array
  int x_block = x/4;

  // bits to "select" the 2 bits of interest
  // from 8 in a uint8_t
  uint8_t bits = 0b11000000 >> 2*hori_offset;
    
  // bits to "select" from
  uint8_t white_bits = 0b00000000;
  uint8_t black_bits = 0b01010101;
  uint8_t red_bits = 0b10101010;
  uint8_t blue_bits = 0b11111111;

  uint8_t and_bits;
  uint8_t or_bits;
    
  // generate 2 uint8_t values:
  // - one to "and" with the element of all_pixel, and
  // - one to "or" with the element of all pixels
  // in order to update the 2 bits of interest appropriately
  if (colour == WHITE) {
    or_bits = bits & white_bits;
    and_bits = ~bits | white_bits;
  } else if (colour == BLACK) {
    or_bits = bits & black_bits;
    and_bits = ~bits | black_bits;
  } else if (colour == RED) {
    or_bits = bits & red_bits;
    and_bits = ~bits | red_bits;
  } else if (colour == BLUE) {
    or_bits = bits & blue_bits;
    and_bits = ~bits | blue_bits;
  }
    
  // bit-wise update of the contents of the all
  // pixels array
  all_pixels[x_block][y] &= and_bits;
  all_pixels[x_block][y] |= or_bits;
}

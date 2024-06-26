#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"


// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!!

#define LED BIT6/* note that bit zero req'd for display */
#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8
#define SWITCHES 15


/* Used to change the colors of the square
*  when you press a button you change this u_int
*  to another color 
*/
u_int colorChange = COLOR_VIOLET;



static char switch_update_interrupt_sense(){
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);/* if switch down, sense up */
  return p2val;
}



void switch_init(){
  P2REN |= SWITCHES;/* enables resistors for switches */
  P2IE |= SWITCHES;/* enable interrupts from switches */
  P2OUT |= SWITCHES;/* pull-ups for switches */
  P2DIR &= ~SWITCHES;/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

// for the switches
void switch_interrupt_handler(){
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}

/* axis zero for col, axis 1 for row
 * index 0 of all the arrays is the x-axis
 * index 2 of all the arrays is the y-axis
 * the colVelocity is used to update the position of the square
 */
short drawPos[2] = {1,1}, controlPos[2] = {2, 2};
short colVelocity = 1, colLimits[2] = {1, screenWidth/2};
short colLimits_2[2] = {1, screenHeight/2};

// draw the square
void draw_ball(int col, int row, unsigned short color){
  fillRectangle(col-1, row-1, 10, 10, color);
}

/* updates the position of the square by drawing a similar square
 * where the square used to be, but color blue to match the color of the screen
 * giving the illusion that it's erased. You only do this if the current position
 * is not the same as the new position.
 */
void screen_update_ball(){
  for (char axis = 0; axis < 2; axis++)
    if (drawPos[axis] != controlPos[axis]) /* position changed? */
      goto redraw;
  return;/* nothing to do */
 redraw:
  draw_ball(drawPos[0], drawPos[1], COLOR_BLUE); /* erase */
  for (char axis = 0; axis < 2; axis++)
    drawPos[axis] = controlPos[axis];
  // you call the fillRectangle with colorChange
  draw_ball(drawPos[0], drawPos[1], colorChange); /* draw */
}

// change the states of each song
// max change state of the songs
char change_state = 0;
char STATE_MAX = 8;

// changes the state machines for the songs
char changeSong = 0;

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;


// state machine changes every second
// it changes the frequencies
// from case 0 to case 7
int song_one(){
  switch(change_state){
  case 0:
    buzzer_set_period(164);
    break;
  case 1:
    buzzer_set_period(329);
    break;
  case 2:
    buzzer_set_period(300);
    break;
  case 3:
    buzzer_set_period(329);
    break;
  case 4:
    buzzer_set_period(400);
    break;
  case 5:
    buzzer_set_period(329);
    break;
  case 6:
    buzzer_set_period(264);
    break;
  case 7:
    buzzer_set_period(100);
    break;
  }
}

// state machine changes every second
// it changes the frequencies
// from case 0 to case 7
int song_two(){
  switch(change_state){
  case 0:
    buzzer_set_period(164);
    break;
  case 1:
    buzzer_set_period(200);
    break;
  case 2:
    buzzer_set_period(300);
    break;
  case 3:
    buzzer_set_period(429);
    break;
  case 4:
    buzzer_set_period(500);
    break;
  case 5:
    buzzer_set_period(729);
    break;
  case 6:
    buzzer_set_period(964);
    break;
  case 7:
    buzzer_set_period(1000);
    break;
  }
}

// state machine changes every second
// it changes the frequencies
// from case 0 to case 7
int song_three(){
  switch(change_state){
  case 0:
    buzzer_set_period(500);
    break;
  case 1:
    buzzer_set_period(1000);
    break;
  case 2:
    buzzer_set_period(400);
    break;
  case 3:
    buzzer_set_period(1000);
    break;
  case 4:
    buzzer_set_period(300);
    break;
  case 5:
    buzzer_set_period(1000);
    break;
  case 6:
    buzzer_set_period(200);
    break;
  case 7:
    buzzer_set_period(1000);
    break;
  }
}

// state machine changes every second
// it changes the frequencies
// from case 0 to case 7
int song_four(){
  switch(change_state){
  case 0:
    buzzer_set_period(500);
    break;
  case 1:
    buzzer_set_period(400);
    break;
  case 2:
    buzzer_set_period(600);
    break;
  case 3:
    buzzer_set_period(300);
    break;
  case 4:
    buzzer_set_period(700);
    break;
  case 5:
    buzzer_set_period(200);
    break;
  case 6:
    buzzer_set_period(800);
    break;
  case 7:
    buzzer_set_period(100);
    break;
  }
}



// change the state
int sec(){
  change_state++;
  if(change_state >= STATE_MAX){
    change_state = 0;
  }
}

void wdt_c_handler(){
  // the switches
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
  // oldCol_x is for the old position on the x-axis that must be updated
  // oldCol_y is for the old position on the y_axis that must be updated
  // index 0 on the arrays is for x, and index 1 is for y
  short oldCol_x = controlPos[0];
  short oldCol_y = controlPos[1];

  static int sec_count = 0; //remember value last time it was called
  // every second change the state
  sec_count++;
  if(sec_count == 100){
    sec_count = 0;
    sec();
  }

  // if S1 is pressed
  if(switches & SW4){  
    // change the color of the square
    colorChange = COLOR_SEA_GREEN;
    changeSong = 4;
    // the new position has to be the x-axis plus 1
    // to move right
    short newCol = oldCol_x + colVelocity;
    if(!(newCol <= colLimits[0]) || !(newCol >= colLimits[1])){
      // redrawScreen needs to be 1 so that the square is redrawn in the new position
      redrawScreen = 1;
      // this will be the new position
      // it doesn't update the square, just the future position
      controlPos[0] = newCol;
    }   
     
  }
  if(switches & SW3){
    colorChange = COLOR_HOT_PINK;
    changeSong = 3;
    // the new position on the x-axis
    // moves left so you subtract 1 from the future position
    // you move backwards towards the negatives
    short newCol = oldCol_x - colVelocity;
    if (!(newCol <= colLimits[0]) || !(newCol >= colLimits[1])){
      redrawScreen = 1;
      controlPos[0] = newCol;
    }
  }
  if(switches & SW2){
    colorChange = COLOR_DARK_VIOLE;
    changeSong = 2;
    // the new position on the y_axis
    // moves down
    // update controlPos[1], the second index
    // because we are changing the y-axis now
    short newCol = oldCol_y + colVelocity;
    // if (!(newCol <= colLimits_2[0])){
      redrawScreen = 1;
      controlPos[1] = newCol;
      // }
  }

  if(switches & SW1){
    colorChange = COLOR_CYAN;
    changeSong = 1;
    // moves up
    short newCol = oldCol_y - colVelocity;
    if (!(newCol <= colLimits_2[0]) || !(newCol >= colLimits_2[1])){
      redrawScreen = 1;
      controlPos[1] = newCol;
    }
  }

}

void update_shape();
void update_song();

void main(){

  P1DIR |= LED;/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);              /**< GIE (enable interrupts) */

  clearScreen(COLOR_BLUE);
  while (1) {/* forever */
    // if redrawScreen is 1 update the shape
    // redrawScreen always turns into 1 when you press a button
    if(redrawScreen) {
      // reset it
      redrawScreen = 0;
      update_shape();
    }
    // play state machine then change states
    switch(changeSong){
    case 1:
      song_one();
      break;
    case 2:
      song_two();
      break;
    case 3:
      song_three();
      break;
    case 4:
      song_four();
      break;
    }
   
    // if(change_song){
    // change_song = 0;
    // song_one();
    // }
    // when you press a button you can see the CPU turning on and off
    P1OUT &= ~LED;/* led off */
    or_sr(0x10);/**< CPU OFF */
    P1OUT |= LED;/* led on */
  }
}

/* it's called everytime the CPU isn't off and when redrawSquare is 1
 * if it's not 1, it does not call it
 */
void update_shape(){
    screen_update_ball();
}

// the interrupt
void __interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;      /* clear pending sw interrupts */
    switch_interrupt_handler();/* single handler for all switches */
  }
}


      

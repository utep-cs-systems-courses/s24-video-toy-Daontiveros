#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"



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
short colLimits_2[2] = {1, screenHeight-10};

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
  for (char axis = 0; axis < 2; axis ++)
    if (drawPos[axis] != controlPos[axis]) /* position changed? */
      goto redraw;
  return;/* nothing to do */
 redraw:
  draw_ball(drawPos[0], drawPos[1], COLOR_BLUE); /* erase */
  for (char axis = 0; axis < 2; axis ++)
    drawPos[axis] = controlPos[axis];
  // you call the fillRectangle with colorChange
  draw_ball(drawPos[0], drawPos[1], colorChange); /* draw */
}


short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;


void wdt_c_handler(){
  // the switches
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
  // oldCol_x is for the old position on the x-axis that must be updated
  // oldCol_y is for the old position on the y_axis that must be updated
  // index 0 on the arrays is for x, and index 1 is for y
  short oldCol_x = controlPos[0];
  short oldCol_y = controlPos[1];
  // if S1 is pressed
  if(switches & SW4) {
    // change the color of the square
    colorChange = COLOR_SEA_GREEN;
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
    // the new position on the y_axis
    // moves down
    // update controlPos[1], the second index
    // because we are changing the y-axis now
    short newCol = oldCol_y + colVelocity;
    if (!(newCol <= colLimits_2[0]) || !(newCol >= colLimits_2[1])){
      redrawScreen = 1;
      controlPos[1] = newCol;
    }
  }

  if(switches & SW1){
    colorChange = COLOR_CYAN;
    // moves up
    short newCol = oldCol_y - colVelocity;
    if (!(newCol <= colLimits_2[0]) || !(newCol >= colLimits_2[1])){
      redrawScreen = 1;
      controlPos[1] = newCol;
    }
  }

}



void update_shape();

void main(){

  P1DIR |= LED;/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);              /**< GIE (enable interrupts) */

  clearScreen(COLOR_BLUE);
  while (1) {/* forever */
    // if redrawScreen is 1 update the shape
    // redrawScreen always turns into 1 when you press a button
    if (redrawScreen) {
      // reset it
      redrawScreen = 0;
      update_shape();
    }
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


      

#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

void f1CarAdvance(MovLayer *ml, Region *fence);

#define SW1 BIT0
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3
int movement;

void switch_interrupt_handler(){
  char p2val = p2sw_read();
  char sw1_down = (p2val & SW1) ? 0 : 1;
  char sw2_down = (p2val & SW2) ? 0 : 1;
  char sw3_down = (p2val & SW3) ? 0 : 1;
  char sw4_down = (p2val & SW4) ? 0 : 1;

  if(sw3_down){
    buzzer_set_period(800);
    movement = -10;
    f1CarAdvance(&ml0,&fieldFence);
  }
  if(sw4_down){ 
    buzzer_set_period(1000);
    movement = 10;
    f1CarAdvance(&ml0,&fieldFence);
  }
}

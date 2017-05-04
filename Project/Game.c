/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "buzzer.h"

#define GREEN_LED BIT6

#define SW1 BIT0
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3

char sw1_down, sw2_down, sw3_down, sw4_down;
int movement;

AbRect car = {abRectGetBounds, abRectCheck, {6,10}}; /**< 10x10 rectangle */
AbRect road = {abRectGetBounds, abRectCheck, {8,5}};
AbRect middle = {abRectGetBounds, abRectCheck,{15, 5}};
AbRArrow rightArrow = {abRArrowGetBounds, abRArrowCheck, 50}; //test

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 10, screenHeight/2 - 10}
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GREEN,
  0
};

Layer f1car = {		/**< Layer with a red square */
  (AbShape *)&car,
  {screenWidth/2, (screenHeight/2) + 50}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GRAY,
  &fieldLayer,
};

Layer layerLeft = {		/**< Layer with an orange circle */
  (AbShape *)&road,
  {(screenWidth/2) + 24, (screenHeight/2) - 50}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE,
  &f1car,
};

Layer layerRight = {		/**< Layer with an orange circle */
  (AbShape *)&road,
  {(screenWidth/2) - 24, (screenHeight/2)-50}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE,
  &layerLeft,
};

Layer layerObjective = {		/**< Layer with an orange circle */
  (AbShape *)&middle,
  {(screenWidth/2), (screenHeight/2)-50}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GREEN,
  &layerRight,
};


/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

MovLayer ml0 = {&f1car, {0,0},0};

/* initial value of {0,0} will be overwritten */
MovLayer ml1 = { &layerLeft, {0,1}, 0};
MovLayer ml2 = { &layerRight, {0,1}, &ml1}; /**< not all layers move */
MovLayer ml3 = { &layerObjective, {0,1}, &ml2}; 

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  

Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis = 1;
  Region shapeBoundary;
  
  Vec2 staticPos = newPos;
  //  int newNewPos = rand() % (screenWidth/2) + 1;
 
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    if (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) {
      //  if(shapeBoundary.botRight.axes[axis] > screenHeight - 10){
      newPos.axes[axis] = -10;
    }	/**< if outside of fence */
    ml->layer->posNext = newPos;
    
  } /**< for ml */
}
void f1CarAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis = 0;
  Region shapeBoundary;
  
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    if(newPos.axes[axis] + movement <screenWidth - 10){
      if(newPos.axes[axis] + movement > 10){
	newPos.axes[axis] = newPos.axes[axis] + movement;
      }
    }
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void scorePoints(MovLayer *f1Car, MovLayer *layerObjective){
  Vec2 newPos;
  u_char axis;
  Region f1CarShapeBoundary;
  Region objectiveShapeBoundary;
  abShapeGetBounds(f1Car->layer->abShape, &f1Car->layer->posNext, &f1CarShapeBoundary);
  for(; layerObjective; layerObjective = layerObjective->next){
    vec2Add(&newPos, &layerObjective->layer->pos, &layerObjective->velocity);
    abShapeGetBounds(layerObjective->layer->abShape, &newPos, &objectiveShapeBoundary);
    if(abShapeCheck(layerObjective->layer->abShape, &layerObjective->layer->pos, &f1CarShapeBoundary.botRight)){
      drawString5x7(screenWidth/2 + 10, screenHeight/2 + 10, "HI", COLOR_WHITE, COLOR_BLUE);
    }	
  }
}

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  //  buzzer_init();
  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();

  layerInit(&layerObjective);
  layerDraw(&layerObjective);


  layerGetBounds(&fieldLayer, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
      switch_interrupt_handler();
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movement = 0;
    //    scorePoints(&ml0, &ml3);
    
    movLayerDraw(&ml0, &f1car);
    movLayerDraw(&ml3, &layerObjective);
  }
}

void switch_interrupt_handler(){
  char p2val = p2sw_read();
  sw1_down = (p2val & SW1) ? 0 : 1;
  sw2_down = (p2val & SW2) ? 0 : 1;
  sw3_down = (p2val & SW3) ? 0 : 1;
  sw4_down = (p2val & SW4) ? 0 : 1;

  if(sw3_down){
    movement = -10;
    f1CarAdvance(&ml0,&fieldFence);
    //    redrawScreen = 1;
    // play_first();
  }
  if(sw4_down){
    movement = 10;
    f1CarAdvance(&ml0,&fieldFence);
    // redrawScreen = 1;
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    f1CarAdvance(&ml0, &fieldFence);
    mlAdvance(&ml3, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}

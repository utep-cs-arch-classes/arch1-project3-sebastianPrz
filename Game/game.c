#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "switches.h"

#define GREEN_LED BIT6

#define SW1 BIT0
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3

char sw1_down , sw2_down, sw3_down, sw4_down;

int movement;

AbRect car = {abRectGetBounds, abRectCheck, {6, 10}};
AbRect road = {abRectGetBounds, abRectCheck, {10, 10}};
AbRect middle ={abRectGetBounds, abRectCheck, {15, 10}};
AbRArrow rightArrow = {abRArrowGetBounds, abRArrowCheck, 50};

AbRectOutline fieldOutline = {
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2 + 10, screenHeight/2 + 10}
};

Layer test = {
  (AbShape *)&rightArrow,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_PINK,
  0
};



Layer fieldLayer = {
  (AbShape *)&fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0},{0,0},
  COLOR_GREEN,
  &test
};

Layer f1car = {
  (AbShape*)&car,
  {screenWidth/2, screenHeight/2 + 60},
  {0,0}, {0,0},
  COLOR_ORANGE,
  &fieldLayer
};

Layer objectiveLeft = {
  (AbShape*) &road,
  {screenWidth/2 - 25, -10},
  {0,0}, {0,0},
  COLOR_RED,
  &f1car
};

Layer objectiveRight = {
  (AbShape*)&road,
  {screenWidth/2 + 25, -10},
  {0,0}, {0,0},
  COLOR_RED,
  &objectiveLeft
};


Layer objective = {
  (AbShape*)&middle,
  {screenWidth/2, -10},
  {0,0}, {0,0},
  COLOR_BLUE,
  &objectiveRight
};

typedef struct MovLayer_s{
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

MovLayer ml0 = {&f1car, {0,0} ,0};

MovLayer ml1 = {&objectiveLeft, {2,3},0};
MovLayer ml2 = {&objectiveRight, {1,1}, &ml1};
MovLayer ml3 = {&objective, {2,1},&ml2};


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

//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}};

void objectiveAdvance(MovLayer *ml, Region *fence){
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for(; ml; ml = ml->next){
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);

    if(shapeBoundary.topLeft.axes[1] > screenHeight - 20){
      newPos.axes[1] = -10;
      ml->velocity.axes[1] += 1;
    }
    
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void steering(MovLayer *ml, Region *fence){
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for(; ml; ml = ml->next){
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);

    if(newPos.axes[0] + movement > 0 && newPos.axes[1] + movement < screenHeight){
      newPos.axes[1] = newPos.axes[1] + movement;
    }
    ml->layer->posNext = newPos;
  }
}
int points = 0;
void scorePoints(MovLayer *ml, MovLayer *f1car){
  Vec2 position;
  Region f1;
  Region objective;
  abShapeGetBounds(f1car->layer->abShape, &f1car->layer->posNext, &f1);
  for(; ml; ml = ml->next){
    vec2Add(&position, &ml->layer->pos, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &position, &objective);
    if(abShapeCheck(f1car->layer->abShape, &f1car->layer->pos, &objective.topLeft)||
       (abShapeCheck(f1car->layer->abShape, &f1car->layer->pos, &objective.botRight))){
	 points += 1;
       }
  }
}


u_int bgColor = COLOR_BLACK;
int redrawScreen = 1;

Region fieldFence;

void main(){
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  //  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();
  
  layerInit(&objectiveRight);
  layerDraw(&objectiveRight);

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
    movLayerDraw(&ml0, &f1car);
    movLayerDraw(&ml3, &objectiveRight);
  }
}

void
switch_interrupt_handler()
{
  char p2val = p2sw_read();
  sw1_down = (p2val & SW1) ? 0 : 1; /* 0 when SW1 is up */
  sw2_down = (p2val & SW2) ? 0 : 1;
  sw3_down = (p2val & SW3) ? 0 : 1;
  sw4_down = (p2val & SW4) ? 0 : 1;

  if(sw3_down){
    movement = -20;
  }
  if(sw4_down){
    movement = 20;
  }
  
}

void wdt_c_handler(){
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    objectiveAdvance(&ml0, &fieldFence);
    
    //    if (p2sw_read())
    //  redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}

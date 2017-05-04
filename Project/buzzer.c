
#include <msp430.h>
#include "buzzer.h"

//defining notes for the songs
#define G2 97
#define Asharp2 116
#define D3 146
#define G3 195
#define F3 174
#define C3 130
#define E3 164
#define C4 261
#define A3 220
#define D4 293
#define Asharp3 233

//defining sequence of notes for playing
int piano1[] = {G2, 0,0,0,0 , Asharp2 , 0,0,0,0, D3, 0,0,0,0, G3, 0,0, G3, 0,0,0,0, F3, 0,0,0,0, C3, 0,0,0,0};
int piano2[] = {C3, 0,0,0,0, E3, 0,0,0,0, G3, 0,0,0,0, C4, 0,0, C4, 0,0,0,0, A3, 0,0,0,0, C3, 0,0,0,0};
int piano3[] = {D3, 0,0,0,0, F3, 0,0,0,0, A3, 0,0,0,0, D3, 0,0, D3, 0,0,0,0, C3, 0,0,0,0, A3, 0,0,0,0};

//auxiliary variable for methods
static int numberOfNotes = 33;
static int currentNote = 0;

//Method that will play the first sequence
void play_first(){                   
  if(currentNote == numberOfNotes){             //when the current note reaches the last one
    currentNote = 0;                     //set it to 0 and repeat
  }
  buzzer_set_period(piano1[currentNote]);//play each note 
  currentNote++;                         //increase the current Note
}
//Method that will play the second sequence
void play_second(){
  if(currentNote == numberOfNotes){
    currentNote = 0;
  }
  buzzer_set_period(piano2[currentNote]);
  currentNote++;
}
//Method that will play the third sequence
void play_third(){
  
  if(currentNote == numberOfNotes){
    currentNote = 0;
  }
  buzzer_set_period(piano3[currentNote]);
  currentNote++;
}

void buzzer_init()
{
    /* 
       Direct timer A output "TA0.1" to P2.6.  
        According to table 21 from data sheet:
          P2SEL2.6, P2SEL2.7, anmd P2SEL.7 must be zero
          P2SEL.6 must be 1
        Also: P2.6 direction must be output
    */
    timerAUpmode();		/* used to drive speaker */
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7; 
    P2SEL |= BIT6;
    P2DIR = BIT6;		/* enable output to speaker (P2.6) */

    // buzzer_advance_frequency();	/* start buzzing!!! */
    //  play_first();
}

void buzzer_set_period(short cycles)
{
  CCR0 = cycles; 
  CCR1 = cycles >> 1;		/* one half cycle */
}

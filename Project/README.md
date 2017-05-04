# Project 3: LCD Game F1 CAR
## Introduction

This game represents a car that has to go in between two obstacles. Once it touches the objective it will add one to the current score and it will display it. If the car does not touch the objective nothing will be added. Button 3 and 4 will move left and right and will make a sound.

At first the shape with the objective and the obstacles will appear from the top and will go down until it reaches the bottom of the screen. To move left you push button 3 and to move right you move button 4.

I received advise from Jose Yanez to implement what I already did on lab 2 in regards to the sound.

## Requirements
Your game should meet the following minimum criteria:

- dynamically render graphical elements that move
-- including one original algorithmically rendered graphical element 
- handle colisions properly with other shapes and area boundaries
- produce sounds triggered by game events
-- in a manner that does not cause the game to pause
- communicate with the player using text
- include a state machine written in assembly language
- respond to user input (from buttons)

Don't forget to properly document your source code and how to play the game.

## Grading Criteria

Your lab will be graded based on the proficiencies it demonstrates.
While we appreciate the complexity of an
arcade quality game, our primary objective is that you demonstrate
course-relevant skills including

- relevant development tools such as make and emacs
- use of timer interrupts to control program timing
- use of switch interrupts to determine when swiches change
- modularization into multiple source files (including header files)
- use of explicit state machines to implement program functionality
- ability to develop or modify (and of course use) linked data structures in c
- mature programming
-- readable, appropriate algorithms, modularization, data structures, symbol names etc

Below is an example of a sample "pong" application that would fully satisfy
requirments.  The graphics would include

- an arena to play in
- a ball
- two paddles (using a shape of your own design)
- a score

And behaviors that include

- the ball moves in 2d, with direction changing when it collides with
  screen elements
- a sound plays when a collision occurs
-- without causing the ball's motion to pause
- scores
 - that advance through multiple rounds of play
 - that the ball either moves in-front-of or behind

## Libraries

Several libraries are provided.  
They can be installed by the default production of Makefile in the repostiory's 
root directory, or by a "$make install" in each of their subdirs.

- timerLib: Provides code to configure Timer A to generate watchdog timer interrupts at 250 Hz

- p2SwLib: Provides an interrupt-driven driver for the four switches on the LCD board and a demo program illustrating its intended functionality.

- lcdLib: Provides low-level lcd control primitives, defines several fonts, 
and a simple demo program that uses them.

- shapeLib: Provides an translatable model for shapes that can be translated 
and rendered as layers.

- circleLib: Provides a circle model as a vector of demi-chord lengths,
pre-computed circles as layers with a variety of radii, 
and a demonstration program that renders a circle.


## Demonstration program

- shape-motion-demo: A demonstration program that uses shapeLib to represent
and render shapes that move.



/*
 * Stepper.h - Stepper library for Wiring/Arduino - Version 1.1.0
 *
 * Original library        (0.1)   by Tom Igoe.
 * Two-wire modifications  (0.2)   by Sebastian Gassner
 * Combination version     (0.3)   by Tom Igoe and David Mellis
 * Bug fix for four-wire   (0.4)   by Tom Igoe, bug fix from Noah Shibley
 * High-speed stepping mod         by Eugene Kozlenko
 * Timer rollover fix              by Eugene Kozlenko
 * Five phase five wire    (1.1.0) by Ryan Orendorff
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Drives a unipolar, bipolar, or five phase stepper motor.
 *
 * When wiring multiple stepper motors to a microcontroller, you quickly run
 * out of output pins, with each motor requiring 4 connections.
 *
 * By making use of the fact that at any time two of the four motor coils are
 * the inverse of the other two, the number of control connections can be
 * reduced from 4 to 2 for the unipolar and bipolar motors.
 *
 * A slightly modified circuit around a Darlington transistor array or an
 * L293 H-bridge connects to only 2 microcontroler pins, inverts the signals
 * received, and delivers the 4 (2 plus 2 inverted ones) output signals
 * required for driving a stepper motor. Similarly the Arduino motor shields
 * 2 direction pins may be used.
 *
 * The sequence of control signals for 5 phase, 5 control wires is as follows:
 *
 * Step C0 C1 C2 C3 C4
 *    1  0  1  1  0  1
 *    2  0  1  0  0  1
 *    3  0  1  0  1  1
 *    4  0  1  0  1  0
 *    5  1  1  0  1  0
 *    6  1  0  0  1  0
 *    7  1  0  1  1  0
 *    8  1  0  1  0  0
 *    9  1  0  1  0  1
 *   10  0  0  1  0  1
 *
 * The sequence of control signals for 4 control wires is as follows:
 *
 * Step C0 C1 C2 C3
 *    1  1  0  1  0
 *    2  0  1  1  0
 *    3  0  1  0  1
 *    4  1  0  0  1
 *
 * The sequence of controls signals for 2 control wires is as follows
 * (columns C1 and C2 from above):
 *
 * Step C0 C1
 *    1  0  1
 *    2  1  1
 *    3  1  0
 *    4  0  0
 *
 * The circuits can be found at
 *
 * http://www.arduino.cc/en/Tutorial/Stepper
 */
#include "millis.h"
// ensure this library description is only included once
#ifndef Stepper_h
#define Stepper_h

// library interface description

  
   
    
    Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
                                 int motor_pin_3, int motor_pin_4);
    

    // speed setter method:
    void setSpeed(long whatSpeed);

    // mover method:
    void step(int number_of_steps);

    int version(void);

  
    void stepMotor(int this_step);
struct  Stepper {
               // Direction of rotation
    unsigned long step_delay; // delay between steps, in ms, based on speed
    int number_of_steps;      // total number of steps this motor can take
    int pin_count;            // how many pins are in use.
            // which step the motor is on

    // motor pin numbers:
    int motor_pin_1;
    int motor_pin_2;
    int motor_pin_3;
    int motor_pin_4;
    

    unsigned long last_step_time; // time stamp in us of when the last step was taken
};

Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
int motor_pin_3, int motor_pin_4)
{
	int step_number = 0;    // which step the motor is on
	int direction = 0;      // motor direction
	last_step_time = 0; // timestamp in us of the last step taken
	number_of_steps = number_of_steps; // total number of steps for this motor
	init_millis(8000000UL); //frequency the atmega328p is running at
	sei();
	// Arduino pins for the motor control connection:
	motor_pin_1 = DDRC | (1<<4);
	motor_pin_2 = DDRC | (1<<5);
	motor_pin_3 = DDRC | (1<<6);
	motor_pin_4 = DDRC | (1<<7);

	// setup the pins on the microcontroller:
	DDRC=motor_pin_1;
	DDRC=motor_pin_2;
	DDRC=motor_pin_3;
	DDRC=motor_pin_4;

	// When there are 4 pins, set the others to 0:
	

	// pin_count is used by the stepMotor() method:
	pin_count = 4;
}
void setSpeed(long whatSpeed)
{
	step_delay = 60L * 1000L * 1000L / number_of_steps / whatSpeed;
}

void step(int steps_to_move)
{
	int steps_left = abs(steps_to_move);  // how many steps to take

	// determine direction based on whether steps_to_mode is + or -:
	if (steps_to_move > 0) { direction = 1; }
	if (steps_to_move < 0) { direction = 0; }


	// decrement the number of steps, moving one step each time:
	while (steps_left > 0)
	{
		unsigned long now = millis()*1000;
		// move only if the appropriate delay has passed:
		if (now - last_step_time >= step_delay)
		{
			// get the timeStamp of when you stepped:
			last_step_time = now;
			// increment or decrement the step number,
			// depending on direction:
			if (direction == 1)
			{
				step_number++;
				if (step_number == number_of_steps) {
					step_number = 0;
				}
			}
			else
			{
				if (step_number == 0) {
					step_number = number_of_steps;
				}
				step_number--;
			}
			// decrement the steps left:
			steps_left--;
			// step the motor to step number 0, 1, ..., {3 or 10}
			if (pin_count == 5)
			stepMotor(step_number % 10);
			else
			stepMotor(step_number % 4);
		}
	}
}
void Stepper::stepMotor(int thisStep)
{
	if (this->pin_count == 4) {
		switch (thisStep) {
			case 0:  // 1010
			PORTC = PORTC | (1<<4);
			PORTC = PORTC | (1<<6);
			PORTC = PORTC | (1<<6);
			PORTC = PORTC | (1<<5);
			break;
			case 1:  // 0110
			PORTC = PORTC | (1<<4);
			PORTC = PORTC | (1<<6);
			PORTC = PORTC | (1<<4);
			PORTC = PORTC | (1<<6);
			break;
			case 2:  //0101
			PORTC = PORTC | (1<<4);
			PORTC = PORTC | (1<<5);
			PORTC = PORTC | (1<<5);
			PORTC = PORTC | (1<<6);
			break;
			case 3:  //1001
			PORTC = PORTC | (1<<4);
			PORTC = PORTC | (1<<6);
			PORTC = PORTC | (1<<5);
			PORTC = PORTC | (1<<4);
			break;
		}
	}
}

#endif


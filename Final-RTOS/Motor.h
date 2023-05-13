//*************************************************************************************
//************************* Ahmed Mohamed Gamal DC-Motor Driver ***********************
//*************************************************************************************

#ifndef _Motor_DC_
#define _Motor_DC_


#include "DIO.h"
//Motor implemented for Port B 

#define MotorPort PORTB

#define Motor1 1
#define Motor2 2

#define Motor1PIN1 P0
#define Motor1PIN2 P1 

#define Motor2PIN3 P2
#define Motor2PIN4 P3



void Motor_Init(char MotorNum);
void Motor_Rotate_Clock(char MotorNum);
void Motor_Rotate_Anit_Clock(char MotorNum);
void Motor_Stop(char MotorNum);

#endif
#include "Motor.h"

void Motor_Init(char MotorNum)
{ 
 switch(MotorNum)
 {
 case Motor1:
   DIO_Init(MotorPort,Motor1PIN1,OUTPUT_PIN);
   DIO_Init(MotorPort,Motor1PIN2,OUTPUT_PIN);
   break;
   
 case Motor2:
   DIO_Init(MotorPort,Motor2PIN3,OUTPUT_PIN);
   DIO_Init(MotorPort,Motor2PIN4,OUTPUT_PIN);
   break;
   
 default:
   break;
 }
}

void Motor_Rotate_Clock(char MotorNum)
{
  switch(MotorNum)
  {
  case Motor1:
    DIO_WritePin(MotorPort,Motor1PIN1,LOGIC_HIGH);
    DIO_WritePin(MotorPort,Motor1PIN2,LOGIC_LOW);
    break;
  case Motor2:
    DIO_WritePin(MotorPort,Motor2PIN3,LOGIC_HIGH);
    DIO_WritePin(MotorPort,Motor2PIN4,LOGIC_LOW);
    break;
  }
}
void Motor_Rotate_Anit_Clock(char MotorNum)
{
  switch(MotorNum)
  {
  case Motor1:
    DIO_WritePin(MotorPort,Motor1PIN1,LOGIC_LOW);
    DIO_WritePin(MotorPort,Motor1PIN2,LOGIC_HIGH);
    break;
  case Motor2:
    DIO_WritePin(MotorPort,Motor2PIN3,LOGIC_LOW);
    DIO_WritePin(MotorPort,Motor2PIN4,LOGIC_HIGH);
    break;
  }
}
void Motor_Stop(char MotorNum)
{
    switch(MotorNum)
  {
  case Motor1:
    DIO_WritePin(MotorPort,Motor1PIN1,LOGIC_LOW);
    DIO_WritePin(MotorPort,Motor1PIN2,LOGIC_LOW);
    break;
  case Motor2:
    DIO_WritePin(MotorPort,Motor2PIN3,LOGIC_LOW);
    DIO_WritePin(MotorPort,Motor2PIN4,LOGIC_LOW);
    break;
  }
}
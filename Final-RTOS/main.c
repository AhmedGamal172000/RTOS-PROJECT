#include <FreeRTOS.h>
#include "task.h"
#include "DIO.h"
#include "FreeRTOSConfig.h"
#include "bitwise_operation.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "Motor.h"
#include <stdio.h>

#define RED_LED 1
#define BLUE_LED 2
#define GREEN_LED 3

#define LOCK_SWITCH 1
#define UNLOCK_SWITCH 0



#define UnLimited 0
#define Limited 1

#define State_UP 1
#define State_Down 2
#define State_LimitedUP 3
#define State_LimtedDown 4

#define State_Nth 0

#define Auto 0 
#define Manual 1

static char SwitchFlag = 0;
static char lock = 0;
static char LimitSwitch = UnLimited;
static char AutoManual = Auto; 
static char GlobalState = State_Nth;

static char UP = 'U';
static char DOWN = 'D';

static short counterSwitchManual = 0;

SemaphoreHandle_t SemaphoreDriver;
SemaphoreHandle_t SemaphoreSwitch;
SemaphoreHandle_t SemaphorePassenger;
SemaphoreHandle_t SemaphoreLimit;
SemaphoreHandle_t SemaphoreTimer;

xQueueHandle xQueueDriver;
xQueueHandle xQueuePassenger;

//------------- Functions Prototypes -------------------------
void vApplicationIdleHook(void);
void TaskSwitch();
void TaskDriver();
void TaskPassenger();
void TaskLimit();
void GPIO_Initializations();
void GPIOA_MY_Handler();
void GPIOF_MY_Handler();
void GPIOE_MY_Handler();

//Pin2 is for Driver UP
//Pin3 is for Driver Down
// ************************* Functions **************************************
void vApplicationIdleHook()
{
	for(;;)
	{
		switch(LimitSwitch)
		{
			case Limited:
				taskENTER_CRITICAL();
				Motor_Stop(Motor1);
				GlobalState = State_Nth;
				LimitSwitch = UnLimited;
				taskEXIT_CRITICAL();
				break;
			case UnLimited:
				break;
			default:
				break;
		}
		switch(AutoManual)
		{
			case Manual:
				taskENTER_CRITICAL();
				Motor_Stop(Motor1);
				AutoManual = Auto;
				GlobalState = State_Nth;
				taskEXIT_CRITICAL();
				break;
			case Auto:
				break;
			default:
				break;
		}
		
		toggle_pin(PORTF,P1);
		delay(50);
	}
}
//**********************************************************************************************

//*************************************** Global PORTA Interrupt *************************************

//**********************************************************************************************
void GPIOA_MY_Handler()
{
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		
	
	//- - - - - - - Driver Up - - - - - - - -
	if(Get_Bit(GPIO_PORTA_MIS_R,P2))
		{
		
			Set_Bit(GPIO_PORTA_ICR_R,P2);
			xSemaphoreGiveFromISR(SemaphoreDriver, &xHigherPriorityTaskWoken);
			xQueueSendToBackFromISR(xQueueDriver,&UP,&xHigherPriorityTaskWoken);
			delay(120);
			while((GPIO_PORTA_DATA_R & 0x04) == 0x04)
			{
				counterSwitchManual++;
				if(counterSwitchManual==1500)
				{
					AutoManual = Manual;
				}
				
			}
			counterSwitchManual =0;
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
			
			//Set_Bit(TIMER0_CTL_R,P0);
			//Set_Bit(TIMER0_CTL_R,P1);
		}
	//- - - - - - - Driver DOWN - - - - - - - -
		else if(Get_Bit(GPIO_PORTA_MIS_R,P3))
		{
			
			Set_Bit(GPIO_PORTA_ICR_R,P3);
			xSemaphoreGiveFromISR(SemaphoreDriver, &xHigherPriorityTaskWoken);
			xQueueSendToBackFromISR(xQueueDriver,&DOWN,&xHigherPriorityTaskWoken);
			delay(120);
			while((GPIO_PORTA_DATA_R & 0x08) == 0x08)
			{
				counterSwitchManual++;
				if(counterSwitchManual==1500)
				{
					AutoManual = Manual;
				}
			}
			counterSwitchManual=0;
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
			//TIMER0_CTL_R |= 0X01; //START TIMER
		}


	//- -  - - - - - Passenger UP - -  - - -  - -  -
		else if(Get_Bit(GPIO_PORTA_MIS_R,P6))
		{
			Set_Bit(GPIO_PORTA_ICR_R,P6);
			xSemaphoreGiveFromISR(SemaphorePassenger,&xHigherPriorityTaskWoken);
			xQueueSendToBackFromISR(xQueuePassenger,&UP,&xHigherPriorityTaskWoken);
			delay(120);
			while(Get_Bit(GPIO_PORTA_DATA_R,P6))
			{
				counterSwitchManual++;
				if(counterSwitchManual==1500)
				{
					AutoManual = Manual;
				}
			}
			counterSwitchManual=0;
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
		}
	//- - -  - - - - Passenger Down - - - - - - -  - -
		
		else if(Get_Bit(GPIO_PORTA_MIS_R,P7))
		{
			Set_Bit(GPIO_PORTA_ICR_R,P7);
			xSemaphoreGiveFromISR(SemaphorePassenger,&xHigherPriorityTaskWoken);
			xQueueSendToBackFromISR(xQueuePassenger,&DOWN,&xHigherPriorityTaskWoken);
			delay(120);
			while(Get_Bit(GPIO_PORTA_DATA_R,P7))
			{
				counterSwitchManual++;
				if(counterSwitchManual==1500)
				{
					AutoManual = Manual;
				}
			}
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
		}
}
void GPIOE_MY_Handler()
{
	//Limiter
	portBASE_TYPE xHigherPriorityTaskWoken2 = pdFALSE;
		//- - - - - - - Switch - - - -  - - - - - -  
	if(Get_Bit(GPIO_PORTE_MIS_R,P1))
		{
			
			Set_Bit(GPIO_PORTE_ICR_R,P1);
			xSemaphoreGiveFromISR(SemaphoreSwitch, &xHigherPriorityTaskWoken2);
			portEND_SWITCHING_ISR(xHigherPriorityTaskWoken2);
		}
	else if(Get_Bit(GPIO_PORTE_MIS_R,P2))
	{
		Set_Bit(GPIO_PORTE_ICR_R,P2);
		xSemaphoreGiveFromISR(SemaphoreLimit,&xHigherPriorityTaskWoken2);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken2);
	}
	//Obstacle
	else if(Get_Bit(GPIO_PORTE_MIS_R,P3))
	{
		UBaseType_t ComeFromISRCritical;
		Set_Bit(GPIO_PORTE_ICR_R,P3);
		//xSemaphoreGiveFromISR(SemaphoreObstacle,&xHigherPriorityTaskWoken2);
		//portEND_SWITCHING_ISR(xHigherPriorityTaskWoken2);
		ComeFromISRCritical = taskENTER_CRITICAL_FROM_ISR();
		if(GlobalState == State_UP)
		{
			DIO_WritePin(PORTF,P2,LOGIC_HIGH);
			Motor_Rotate_Anit_Clock(Motor1);
			delay(1500);
			Motor_Stop(Motor1);
			GlobalState = State_Nth;
			DIO_WritePin(PORTF,P2,LOGIC_LOW);
		}
		taskEXIT_CRITICAL_FROM_ISR(ComeFromISRCritical);
	}
}

//*************************************** Limit Switch *********************************
void TaskLimit()
{
	xSemaphoreTake(SemaphoreLimit,0);
	for(;;)
	{
		xSemaphoreTake(SemaphoreLimit,portMAX_DELAY);	
		taskENTER_CRITICAL();
		switch(GlobalState)
		{
			case State_UP:
				GlobalState = State_LimitedUP;
				break;
			case State_Down:
				GlobalState = State_LimtedDown;
				break;
		}
		LimitSwitch = Limited;
		Motor_Stop(Motor1);
		LimitSwitch = UnLimited;
		taskEXIT_CRITICAL();
	}
}
//*************************************** Switch ON/OFF *************************************
void TaskSwitch()
{

	xSemaphoreTake(SemaphoreSwitch,0);
	for(;;)
	{
		xSemaphoreTake(SemaphoreSwitch,portMAX_DELAY);
		taskENTER_CRITICAL();
		switch(Get_Bit(GPIO_PORTE_DATA_R,P1))
		{
			case 1:
				SwitchFlag = LOCK_SWITCH;
				break;
			case 0:
				SwitchFlag = UNLOCK_SWITCH;
				break;
			default:
				break;
		}
		taskEXIT_CRITICAL();
	}
}

//*************************************** Driver *************************************
void TaskDriver()
{
	static char ValueRecieved;
	xSemaphoreTake(SemaphoreDriver,0);
	for(;;)
	{
		xSemaphoreTake(SemaphoreDriver,portMAX_DELAY);
		xQueueReceive(xQueueDriver,&ValueRecieved,0);

		taskENTER_CRITICAL();
			switch(ValueRecieved)
			{
				case 'U':
					if(GlobalState != State_LimitedUP)
						{
							GlobalState = State_UP;
							DIO_WritePin(PORTF,P3,LOGIC_HIGH);
							Motor_Rotate_Clock(Motor1);
						}
					break;
				case 'D':
					if(GlobalState !=State_LimtedDown)
						{
							GlobalState = State_Down;
						DIO_WritePin(PORTF,P3,LOGIC_LOW);
						Motor_Rotate_Anit_Clock(Motor1);					
						}
					break;
				default:
					break;
			
		}
			taskEXIT_CRITICAL();
	}
}
//*************************************** PASSENGER *************************************
void TaskPassenger()
{
	static char ValueRecievedPassenger;
	xSemaphoreTake(SemaphorePassenger,0);
	for(;;)
	{
		xSemaphoreTake(SemaphorePassenger,portMAX_DELAY);
		xQueueReceive(xQueuePassenger,&ValueRecievedPassenger,0);
		if(SwitchFlag == 0)
		{
			taskENTER_CRITICAL();
			switch(ValueRecievedPassenger)
			{
				case 'U':
					if(GlobalState != State_LimitedUP)
						{
							GlobalState = State_UP;
							DIO_WritePin(PORTF,P3,LOGIC_HIGH);
							Motor_Rotate_Clock(Motor1);
						}
					break;
				case 'D':
					if(GlobalState !=State_LimtedDown)
						{
							GlobalState = State_Down;
							DIO_WritePin(PORTF,P3,LOGIC_LOW);
							Motor_Rotate_Anit_Clock(Motor1);
						}
					break;
				default:
					break;
			}
			taskEXIT_CRITICAL();
		}
		else
		{
			//Nothing
		}
	}
}
void GPIO_Initializations()
{
	DIO_Init(PORTF,P0,INPUT_PIN);
	DIO_Init(PORTF,RED_LED,OUTPUT_PIN);
	DIO_Init(PORTF,BLUE_LED,OUTPUT_PIN);
	DIO_Init(PORTF,GREEN_LED,OUTPUT_PIN);
	DIO_Init(PORTF,P4,INPUT_PIN);
	
	DIO_Init(PORTA,P2,INPUT_PIN);
	DIO_Init(PORTA,P3,INPUT_PIN);
	DIO_Init(PORTA,P4,INPUT_PIN);
	DIO_Init(PORTA,P6,INPUT_PIN);
	DIO_Init(PORTA,P7,INPUT_PIN);
	
	DIO_Init(PORTE,P1,INPUT_PIN);
	DIO_Init(PORTE,P2,INPUT_PIN);
	DIO_Init(PORTE,P3,INPUT_PIN);
	
	DIO_InterruptInit(PORTA,P2);
	DIO_InterruptInit(PORTA,P3);
	DIO_InterruptInit(PORTA,P4);
	DIO_InterruptInit(PORTA,P6);
	DIO_InterruptInit(PORTA,P7);
	
	DIO_InterruptInit(PORTE,P1);
	DIO_InterruptInit(PORTE,P2);
	DIO_InterruptInit(PORTE,P3);
	
	DIO_InterruptInit(PORTF,P0);
	DIO_InterruptInit(PORTF,P4);
}
int main()
{ //DIO_init & interrupt
	
	GPIO_Initializations();
	//Motor Init
	Motor_Init(Motor1);
	//QUEUE
	xQueueDriver = xQueueCreate(1,sizeof(char));
	xQueuePassenger = xQueueCreate(1,sizeof(char));
	
	//semaphore Driver 
	vSemaphoreCreateBinary(SemaphoreDriver);   // <----------------
	vSemaphoreCreateBinary(SemaphoreSwitch);
	vSemaphoreCreateBinary(SemaphorePassenger);
	vSemaphoreCreateBinary(SemaphoreLimit);
	if(SemaphoreDriver != NULL)
	{

		xTaskCreate(TaskPassenger,"TaskPassenger",240,NULL,3,NULL);
		xTaskCreate(TaskDriver,"TaskDriver",240,NULL,2,NULL);
		xTaskCreate(TaskSwitch,"TaskSwitch",240,NULL,1,NULL);
		xTaskCreate(TaskLimit,"TaskLimit",240,NULL,4,NULL);
		
		vTaskStartScheduler();
	}
	for(;;)
	{
		
	}
}
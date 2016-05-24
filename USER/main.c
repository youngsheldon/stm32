#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "usart2.h"
#include "usart3.h"
#include "oled.h" 
#include "ui.h"
#include "timer.h"
#include "net.h"
#include "string.h"
#include "stdlib.h"
#include "stm32f10x_it.h" 
//hi,i am geek!
int main(void)
{
	delay_init();	    	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	Uart2_Init(115200);
	OLED_Init();
	LED_Init();
	OLED_Clear(); 	
	KEY_Init();//IO��ʼ��
	TIM3_Int_Init(499,7199);//10Khz�ļ���Ƶ�ʣ�������500Ϊ50ms  
	NET_Init();
	ConnectToDevice();
	delay_ms(300);
	LED2 = 0;
	delay_ms(100);
	LED2 = 1;
	EEPROM_Init();
	// ReadLedStatusFromFlash();
	StartToUploadFlag = 1;
	while(1) 
	{		
		MainMenuIntoSubMenu();
		SwitchDetect();
	}
}


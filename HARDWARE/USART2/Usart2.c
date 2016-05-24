#include "stm32f10x.h"
#include "stm32f10x_it.h" 
#include "usart2.h"
#include "led.h"
#include "usart.h"	
#include <string.h>
#include <stdlib.h>
#include "timer.h"

//���ݲ�ֺ궨�壬�ڷ��ʹ���1�ֽڵ���������ʱ������int16��float�ȣ���Ҫ�����ݲ�ֳɵ����ֽڽ��з���
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)		) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

static u8 data_to_send[50];	//�������ݻ���
u8 SaveRxBuffer[600];
int RxCompeleteFlag = 0;
int RXCouter = 0;

void Uart2_Init(u32 br_num)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART2��GPIOAʱ��
    //USART3_TX   GPIOB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //USART3_RX	  GPIOB11��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure); 

    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    //USART ��ʼ������
    USART_InitStructure.USART_BaudRate = br_num;//���ڲ�����
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART2, &USART_InitStructure); //��ʼ������1
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�(����1�ֽ�����)
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//�������ڽ����ж�(����1֡����)
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���1 
}
static u8 TxBuffer[256];
static u8 TxCounter=0;
static u8 count=0;

void Uart2_Put_Char(unsigned char DataToSend)
{
	TxBuffer[count++] = DataToSend;  
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE); 
}

void Uart2_IRQ(void)
{
    u8 data;
	if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
    {
        USART_ReceiveData(USART2);
    }
		
	//�����ж�
	if((USART2->SR & (1<<7))&&(USART2->CR1 & USART_CR1_TXEIE))
	{
		USART2->DR = TxBuffer[TxCounter++]; //дDR����жϱ�־          
		if(TxCounter == count)
		{
			USART2->CR1 &= ~USART_CR1_TXEIE;		//�ر�TXE�ж�
			USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
		}
	}
	//�����ж� (���ռĴ����ǿ�) 
	if(USART2->SR & (1<<5))
	{
        SaveRxBuffer[RXCouter++] = USART2->DR;
	}
    else if(USART_GetITStatus(USART2,USART_IT_IDLE) != RESET)
    {
        data = USART2->SR;
        data = USART2->DR;
        RxCompeleteFlag = 1;
    }
}

void Uart2_SendPackt(unsigned char *Str,int len)
{
    int i = 0;

    for(;i<len;i++)
        Uart2_Put_Char(*(Str+i));
}

void Uart2_Put_String(unsigned char *Str)
{
	//�ж�Strָ��������Ƿ���Ч.
	while(*Str)
	{
	//�Ƿ��ǻس��ַ� �����,������Ӧ�Ļس� 0x0d 0x0a
	if(*Str=='\r')Uart2_Put_Char(0x0d);
		else if(*Str=='\n')Uart2_Put_Char(0x0a);
			else Uart2_Put_Char(*Str);
	//ָ��++ ָ����һ���ֽ�.
	Str++;
	}
}

void Uart2SendDataOut(u8 *DataToSend , int data_num)
{
	u8 i;
	for(i=0;i<data_num;i++)
		TxBuffer[count++] = *(DataToSend+i);
	if(!(USART2->CR1 & USART_CR1_TXEIE))
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE); 
}



#ifndef FSTOS_OS_H
#define FSTOS_OS_H

#include "task.h"
#include "usart.h"	 
#include "sys.h"
#include "config.h"

//�ӿ�
void FSTOS_init(void);   //��ʼ��ϵͳ
void FSTOS_start(void);  //��ʼ�������

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

//�ڲ�����
void PendSV_init(void); 
void SysTick_Init(void);


#endif //FSTOS_OS_H

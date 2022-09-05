#ifndef FSTOS_OS_H
#define FSTOS_OS_H

#include "task.h"
#include "usart.h"	 
#include "sys.h"
#include "config.h"

//接口
void FSTOS_init(void);   //初始化系统
void FSTOS_start(void);  //开始任务调度

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

//内部函数
void PendSV_init(void); 
void SysTick_Init(void);


#endif //FSTOS_OS_H

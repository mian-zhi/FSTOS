#ifndef FSTOS_TASK_H
#define FSTOS_TASK_H

#include <stdint.h>
#include "usart.h"
#include "fstos.h"

#define CONFIG_MAX_TASK_NUM 8

typedef uint32_t stack_t;

typedef struct{
	void *stack;
	uint32_t delay_ticks;
}Task_Control_Block_t;  

//全局变量
extern Task_Control_Block_t tcb_list[];      //任务列表
extern Task_Control_Block_t *current_TCB;    //当前任务控制块指针

extern uint16_t current_task_id;             //当前任务id号
extern uint16_t next_task_id;                //下一个任务id号

extern uint16_t max_task_num;                //最大任务数

extern uint16_t is_first_switch_task;        //标志位，是否是第一次切换
extern uint8_t task_switch_enable;           //任务调度器是否使能

extern uint32_t now_tick;											//系统时间，由Systick来自加

extern void asmprint(void);

//接口
extern void switch_current_TCB(void);
void Iint_task(void);
uint16_t Creat_task(void *function , void *arguments , stack_t *stack , int stack_size);
void os_sleep_us(uint32_t us);
void os_sleep_ms(uint32_t ms);
	

//内部函数
void task_idle(void);
void switch_task(void);


#endif //FSTOS_TASK_H

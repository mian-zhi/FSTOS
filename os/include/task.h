#ifndef FSTOS_TASK_H
#define FSTOS_TASK_H

#include <stdint.h>
#include "sys.h"
#include "usart.h"

#include "config.h"
#include "fstos.h"

#define CONFIG_MAX_TASK_NUM 8

typedef uint32_t stack_t;

//任务链表结点声明
typedef struct{
	int TCB_id;
	struct Task_Control_Block_t * TCB_ptr;  //任务控制块指针
	struct node *next;                      //指向下一个节点
}node;

typedef struct{
	void *stack;            //栈顶指针
	stack_t *stack_buttom;  //栈底指针
	int task_stack_size;    //栈大小
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

//小功能
void print_stack(void); //打印剩余堆栈

//内部函数
void task_idle(void);
void switch_task(void);


#endif //FSTOS_TASK_H

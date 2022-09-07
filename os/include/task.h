#ifndef FSTOS_TASK_H
#define FSTOS_TASK_H

#include <stdint.h>
#include "sys.h"
#include "usart.h"

#include "config.h"
#include "fstos.h"

#define CONFIG_MAX_TASK_NUM 8

typedef uint32_t stack_t;

//��������������
typedef struct{
	int TCB_id;
	struct Task_Control_Block_t * TCB_ptr;  //������ƿ�ָ��
	struct node *next;                      //ָ����һ���ڵ�
}node;

typedef struct{
	void *stack;            //ջ��ָ��
	stack_t *stack_buttom;  //ջ��ָ��
	int task_stack_size;    //ջ��С
	uint32_t delay_ticks;
}Task_Control_Block_t; 


//ȫ�ֱ���
extern Task_Control_Block_t tcb_list[];      //�����б�
extern Task_Control_Block_t *current_TCB;    //��ǰ������ƿ�ָ��

extern uint16_t current_task_id;             //��ǰ����id��
extern uint16_t next_task_id;                //��һ������id��

extern uint16_t max_task_num;                //���������

extern uint16_t is_first_switch_task;        //��־λ���Ƿ��ǵ�һ���л�
extern uint8_t task_switch_enable;           //����������Ƿ�ʹ��

extern uint32_t now_tick;											//ϵͳʱ�䣬��Systick���Լ�

extern void asmprint(void);

//�ӿ�
extern void switch_current_TCB(void);
void Iint_task(void);
uint16_t Creat_task(void *function , void *arguments , stack_t *stack , int stack_size);
void os_sleep_us(uint32_t us);
void os_sleep_ms(uint32_t ms);

//С����
void print_stack(void); //��ӡʣ���ջ

//�ڲ�����
void task_idle(void);
void switch_task(void);


#endif //FSTOS_TASK_H

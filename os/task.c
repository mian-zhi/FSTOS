#include "task.h"
#include "sys.h"
#include "config.h"

//���������б�
Task_Control_Block_t tcb_list[CONFIG_MAX_TASK_NUM];
//���嵱ǰ����ָ��
Task_Control_Block_t *current_TCB;

//���嵱ǰ�����ID
uint16_t current_task_id = 0;
//�����б�����һ���������һ������id
uint16_t next_task_id = 0;

//�����˿�������
uint16_t max_task_num = CONFIG_MAX_TASK_NUM + 1;
//��־λ����־�Ƿ��ǵ�һ���л�����
uint16_t is_first_switch_task = 1;

//��־λ���Ƿ�ʹ���������л��������ӳٺ����йر����������
uint8_t task_switch_enable = 1;
//���ڼ�¼���ڵ�ʱ�䣬�����ӳٺ���
uint32_t now_tick = 0;

#define STACK_IDLE_SIZE 32
stack_t stack_idle[STACK_IDLE_SIZE];
/******************************************************
�������ܣ���ʼ��������ƣ�
          ����һ����������
					��ʼ����ǰ������ƿ�ָ��
					�����̶�ջָ��PSP����Ϊ0
					��PSP��ջֻ��ʹ��MSRָ��ͨ���Ĵ������أ�
�������ƣ�Iint_task
������������
��������ֵ����
*******************************************************/
void Iint_task()
{
	Creat_task(task_idle,0,stack_idle,STACK_IDLE_SIZE);
	current_TCB = &tcb_list[0];
	__asm{
		MOV R0 , 0X0
		MSR PSP , R0
	}
}

uint32_t idle_ticks = 0;
void task_idle()
{
	for(;;) idle_ticks++;
}

/******************************************************
�������ܣ�����һ�����񣬲�����ջ�ͺ������
�������ƣ�Creat_task
����������stack_t *stack����������ջָ��
          stack_size��ջ��С
					void *function��������õĺ���
					void *arguments��������õĺ����Ĳ���
��������ֵ��next_task_id����һ�����������ţ�ȫ�ֱ�����
*******************************************************/
uint16_t Creat_task(void *function , void *arguments , stack_t *stack , int stack_size)
{
	//����ջ��������Cotex-M3������ջ���������������������Ҫ��ջ��ִ��ָ��ջ�����
	//�����������ȼ���push������Ҫָ���������һ��Ԫ�ص���һ��Ԫ�أ���Ȼ�ǷǷ��ģ��������ǲ�������������
	stack_t *stack_top = &stack[stack_size];
	
	//�ж��Ƿ񳬹�����������
	if(next_task_id>CONFIG_MAX_TASK_NUM) return 0;
	
	//��ʼ��ջ��Ԥ���ṩһ����Ϊ�Ļ���
	//ͬʱ�������Ҫ���õĺ���ָ�봫��PC�Ĵ����У�������������R0�Ĵ���
	//auto save by Cotex-M3
	*(--stack_top) = (stack_t)0x01000000u;  //xPSR Bit24 = 1
	*(--stack_top) = (stack_t)function;     //R15 PC function entry point
	*(--stack_top) = (stack_t)0x14141414u;  //R14 LR
	*(--stack_top) = (stack_t)0x12121212u;  //R12
	*(--stack_top) = (stack_t)0x03030303u;  //R3
	*(--stack_top) = (stack_t)0x02020202u;  //R2
	*(--stack_top) = (stack_t)0x01010101u;  //R1
	*(--stack_top) = (stack_t)arguments;  //R0
	//manual save by developer
	*(--stack_top) = (stack_t)0x11111111u; // R11
	*(--stack_top) = (stack_t)0x10101010u; // R10
	*(--stack_top) = (stack_t)0x09090909u; // R9
	*(--stack_top) = (stack_t)0x08080808u; // R8
	*(--stack_top) = (stack_t)0x07070707u; // R7
	*(--stack_top) = (stack_t)0x06060606u; // R6
	*(--stack_top) = (stack_t)0x05050505u; // R5
	*(--stack_top) = (stack_t)0x04040404u; // R4
	tcb_list[next_task_id].stack = stack_top;

	return next_task_id++;
}	

//�л�����ǰ���ƿ�
void switch_current_TCB()
{
		int i = 0;
		if (current_task_id >= next_task_id - 1) 
		{
        // check if not create any task, switch to idle task
        if (next_task_id >= 2)   //�Ѿ�ִ�е���ִ�������б�����һ��
				{
            current_task_id = 1; //���ص���һ����������ִ��
        } 
				else //û�д����κ�����ִ�п�������
				{
            current_task_id = 0;
            current_TCB = &tcb_list[0];  //ִ�е�0�����񣬾��ǿ�������
            return;
        }
    } 
		else 
		{
        ++current_task_id; //ѡ����һ������
    }
		//current_TCB = &tcb_list[current_task_id];
		current_TCB = &tcb_list[0];
    for (i = current_task_id; i < next_task_id; ++i) {
        Task_Control_Block_t *checking_TCB = &tcb_list[i];
        // BUG!! now ticks is not update
				//�ҵ��������������ӳٵ�������ӳٽ���������
        if (checking_TCB->delay_ticks == 0 || checking_TCB->delay_ticks <= now_tick) {
            checking_TCB->delay_ticks = 0;
            current_TCB = checking_TCB;
            current_task_id = i;
            return;
        }				
    }
}

//����˯�ߺ���
void os_sleep_us(uint32_t us) {
    uint32_t delay_ticks = us / CONFIG_OS_TICK_TIME_US;   //������Ҫ���߶��ٸ�ʱ��Ƭ
    uint32_t target_tick = now_tick + delay_ticks;        //��������ߵ��ľ���ʱ��
    current_TCB->delay_ticks = target_tick;               //��¼���߽���ʱ�䵽������ƿ���
    switch_task();                                        //����һ�ε���
}

//����˯�ߺ�����msΪ��λ
void os_sleep_ms(uint32_t ms) {
		os_sleep_us(ms * 1000);
}

//Systick�жϺ��� ��Ҫ��������¼ϵͳʱ�䣬��ʱ��������PendSV�ж�
void SysTick_Handler(void)
{	
	//printf("11");
	now_tick++;//10ms
	if (task_switch_enable) {
			switch_task();
	}
	//if(now_tick%100 == 0) switch_task();
}

//void PendSV_Handler()
//{
//	printf("1");
//}

/***************
������ຯ����PenSV�жϷ�����
����ֳ��������ͻָ��ֳ��Ĺ���
***************/
__asm void PendSV_Handler( void )
{
	  IMPORT tcb_list
    IMPORT current_TCB
    IMPORT current_task_id
    IMPORT next_task_id
    IMPORT max_task_num
    IMPORT switch_current_TCB
    IMPORT is_first_switch_task
        
    IMPORT now_tick

    PRESERVE8  
	
		CPSID I

    MRS R0, PSP
    CBZ R0, restore_context

save_context
    MRS R0, PSP
    STMDB R0!, {R4-R11}
    LDR R1, =current_TCB
    LDR R1, [R1]
    STR R0, [R1]

select_next_TCB
    PUSH {LR}
    BL switch_current_TCB
    POP {LR}

restore_context
    LDR R0, =current_TCB
    LDR R0, [R0]
    LDR R0, [R0]
    LDMIA R0!, {R4-R11}
    MSR PSP, R0
    ORR LR, LR, #0x4 

    CPSIE I

    BX LR

    ENDP
    
    NOP
}

//����PendSV�жϣ���PendSV���������ڷ���һ�ε���
void switch_task() {
    // set pendsv
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; 
}

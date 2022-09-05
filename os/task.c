#include "task.h"
#include "sys.h"
#include "config.h"

//定义任务列表
Task_Control_Block_t tcb_list[CONFIG_MAX_TASK_NUM];
//定义当前任务指针
Task_Control_Block_t *current_TCB;

//定义当前任务的ID
uint16_t current_task_id = 0;
//任务列表的最后一个任务的下一个任务id
uint16_t next_task_id = 0;

//算上了空闲任务
uint16_t max_task_num = CONFIG_MAX_TASK_NUM + 1;
//标志位，标志是否是第一次切换任务
uint16_t is_first_switch_task = 1;

//标志位，是否使能了任务切换，用于延迟函数中关闭任务调度器
uint8_t task_switch_enable = 1;
//用于记录现在的时间，用于延迟函数
uint32_t now_tick = 0;

#define STACK_IDLE_SIZE 32
stack_t stack_idle[STACK_IDLE_SIZE];
/******************************************************
函数功能：初始化任务控制，
          创建一个空闲任务
					初始化当前任务控制块指针
					将进程堆栈指针PSP设置为0
					（PSP堆栈只能使用MSR指令通过寄存器加载）
函数名称：Iint_task
函数参数：无
函数返回值：无
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
函数功能：创建一个任务，并分配栈和函数入口
函数名称：Creat_task
函数参数：stack_t *stack：任务分配的栈指针
          stack_size：栈大小
					void *function：任务调用的函数
					void *arguments：任务调用的函数的参数
函数返回值：next_task_id：下一个任务的任务号（全局变量）
*******************************************************/
uint16_t Creat_task(void *function , void *arguments , stack_t *stack , int stack_size)
{
	//任务栈顶，由于Cotex-M3是满减栈，自上往下生长，因此需要将栈顶执政指到栈的最后
	//而且由于是先减再push，所有要指到数组最后一个元素的下一个元素（虽然是非法的，但是我们并不解引用它）
	stack_t *stack_top = &stack[stack_size];
	
	//判断是否超过了任务上限
	if(next_task_id>CONFIG_MAX_TASK_NUM) return 0;
	
	//初始化栈，预先提供一个人为的环境
	//同时将任务的要调用的函数指针传入PC寄存器中，函数参数传入R0寄存器
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

//切换任务当前控制块
void switch_current_TCB()
{
		int i = 0;
		if (current_task_id >= next_task_id - 1) 
		{
        // check if not create any task, switch to idle task
        if (next_task_id >= 2)   //已经执行到可执行任务列表的最后一个
				{
            current_task_id = 1; //返回到第一个任务重新执行
        } 
				else //没有创建任何任务，执行空闲任务
				{
            current_task_id = 0;
            current_TCB = &tcb_list[0];  //执行第0个任务，就是空闲任务
            return;
        }
    } 
		else 
		{
        ++current_task_id; //选择下一个任务
    }
		//current_TCB = &tcb_list[current_task_id];
		current_TCB = &tcb_list[0];
    for (i = current_task_id; i < next_task_id; ++i) {
        Task_Control_Block_t *checking_TCB = &tcb_list[i];
        // BUG!! now ticks is not update
				//找到就绪的任务（无延迟的任务和延迟结束的任务）
        if (checking_TCB->delay_ticks == 0 || checking_TCB->delay_ticks <= now_tick) {
            checking_TCB->delay_ticks = 0;
            current_TCB = checking_TCB;
            current_task_id = i;
            return;
        }				
    }
}

//任务睡眠函数
void os_sleep_us(uint32_t us) {
    uint32_t delay_ticks = us / CONFIG_OS_TICK_TIME_US;   //计算需要休眠多少个时间片
    uint32_t target_tick = now_tick + delay_ticks;        //计算出休眠到的绝对时间
    current_TCB->delay_ticks = target_tick;               //记录休眠结束时间到任务控制块中
    switch_task();                                        //发起一次调度
}

//任务睡眠函数，ms为单位
void os_sleep_ms(uint32_t ms) {
		os_sleep_us(ms * 1000);
}

//Systick中断函数 主要工作：记录系统时间，定时进行悬起PendSV中断
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
内联汇编函数，PenSV中断服务函数
完成现场保护、和恢复现场的工作
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

//悬起PendSV中断，等PendSV来处理，等于发起一次调度
void switch_task() {
    // set pendsv
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; 
}

#include <fstos.h>

#define SYSTICK_LOAD (CONFIG_OS_TICK_TIME_US * CONFIG_OS_SYSTICK_CLK - 1)

//初始化系统
void FSTOS_init()
{
	init_memory();
	Iint_task();
}

//初始化定时器和两个中断
void FSTOS_start()
{
	SysTick_Init();
	PendSV_init();
}


//初始化PendSV中断，设置其优先级
void PendSV_init(void) {
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
}

//初始化内部SysTick定时器及其中断
void SysTick_Init(void)
{
    SysTick->LOAD  = SYSTICK_LOAD;
    SysTick->VAL   = 0;
    SysTick->CTRL  |= // SysTick_CTRL_CLKSOURCE_Msk |
                      SysTick_CTRL_TICKINT_Msk   |
                      SysTick_CTRL_ENABLE_Msk	|
                      0; 
    NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
}


//延迟函数，在这个过程中，直接关闭任务调度器，直到延迟结束才重新开放
void delay_us(uint32_t us) {
	
    const uint32_t load = us * CONFIG_OS_SYSTICK_CLK - 1;  //计算装载值
	
    uint8_t old_switch_task_enable = task_switch_enable;
    uint32_t old_SysTick_val = SysTick->VAL;
    task_switch_enable = 0;   //直接关闭任务调度器

    SysTick->LOAD = load;  //载入装载值
    
    while (SysTick->VAL) {  //等待
    }
    
    task_switch_enable = old_switch_task_enable;  //开启任务调度
    SysTick->LOAD = SYSTICK_LOAD;                 //恢复原先的装载值
    SysTick->VAL = 0;                             //恢复原先的计数值
}

void delay_ms(uint32_t ms) {
	delay_us(1000 * ms);
}

#include <fstos.h>

#define SYSTICK_LOAD (CONFIG_OS_TICK_TIME_US * CONFIG_OS_SYSTICK_CLK - 1)

//��ʼ��ϵͳ
void FSTOS_init()
{
	init_memory();
	Iint_task();
}

//��ʼ����ʱ���������ж�
void FSTOS_start()
{
	SysTick_Init();
	PendSV_init();
}


//��ʼ��PendSV�жϣ����������ȼ�
void PendSV_init(void) {
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
}

//��ʼ���ڲ�SysTick��ʱ�������ж�
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


//�ӳٺ���������������У�ֱ�ӹر������������ֱ���ӳٽ��������¿���
void delay_us(uint32_t us) {
	
    const uint32_t load = us * CONFIG_OS_SYSTICK_CLK - 1;  //����װ��ֵ
	
    uint8_t old_switch_task_enable = task_switch_enable;
    uint32_t old_SysTick_val = SysTick->VAL;
    task_switch_enable = 0;   //ֱ�ӹر����������

    SysTick->LOAD = load;  //����װ��ֵ
    
    while (SysTick->VAL) {  //�ȴ�
    }
    
    task_switch_enable = old_switch_task_enable;  //�����������
    SysTick->LOAD = SYSTICK_LOAD;                 //�ָ�ԭ�ȵ�װ��ֵ
    SysTick->VAL = 0;                             //�ָ�ԭ�ȵļ���ֵ
}

void delay_ms(uint32_t ms) {
	delay_us(1000 * ms);
}

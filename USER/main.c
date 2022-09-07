#include "sys.h"
#include "usart.h"

#include "fstos.h"
#include "task.h"

#include "led.h" 
#include "key.h" 
#include "stdint.h"


#define STACK_SIZE 256

stack_t stack_0[STACK_SIZE];
stack_t stack_1[STACK_SIZE];

void task1()
{
	int a[5] = {1,2,3,4,5};
	for(;;)
	{
		print_stack();
		printf("task1\r\n");
		os_sleep_ms(1000);
	}
}

void task2()
{	
	for(;;)
	{
		print_stack();
		printf("task2\r\n");
		os_sleep_ms(2000);
	}
}

int main(void)
{			
	Stm32_Clock_Init(9); //系统时钟设置
	uart_init(72,9600);	 //串口初始化为9600

	FSTOS_init();

	Creat_task(task1,0,stack_0,STACK_SIZE);
	Creat_task(task2,0,stack_1,STACK_SIZE);
	
	FSTOS_start();
	
	while(1)
	{
		;
	}
}


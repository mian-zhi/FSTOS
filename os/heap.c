#include "heap.h"

//定义内存块节点
struct memory_node{
	int size;        //内存块大小
	int offset;      //内存块偏移量
	char used;       //内存块是否被使用
	struct memory_node *prev;   //指向上一个内存块的地址
	struct memory_node *next;   //指向下一个内存块的地址
};

typedef struct memory_node memory_node_t;

char heap[CONFIG_OS_HEAP_SIZE + sizeof(memory_node_t)];//定义堆栈

memory_node_t *memory_all;

void init_memory()
{
	memory_all = (memory_node_t *)heap;
	memory_all->size = CONFIG_OS_HEAP_SIZE;
	memory_all->offset = 0;
	memory_all->used = 0;
	memory_all->prev = 0;
	memory_all->next = 0;
	PrintHearStatus();
}

void *OSmalloc(int size)
{
	void *new_memory;
	memory_node_t *node = memory_all;
	while(node)//遍历内存块节点
	{
		if(node->used == 0)//是否被使用
		{
			int size_with_info = size + sizeof(memory_node_t);  //计算所需要的内存大小
			int remain = node->size - size_with_info;           //计算该内存块被使用后的剩余空间
			if(remain >= 0){                                     //如果大于0
				node->size = size;
				node->used = 1;
				if(remain>sizeof(memory_node_t))
				{
					int info_offset = node->offset + size_with_info;  //计算下一块内存块相对堆起点的偏移地址
																														//偏移地址 = 该内存块的偏移地址 + 申请所需要的地址
					memory_node_t *next_memory_node = (memory_node_t *)&heap[info_offset];
																														//得到下一个内存块的实际地址
					
					//这里是下一块内存块的信息了
					//原本的 node 指向的内存块，被我申请后，剩下的内存块
					next_memory_node->size = remain - sizeof(memory_node_t);
					next_memory_node->offset = info_offset + sizeof(memory_node_t);  //内存偏移 = 其他内存块占用的 + 我本身内存块的信息节点占用的内存
					next_memory_node->used = 0;
					
					//将该内存块插入原本的链表中
					next_memory_node->prev = node;
					next_memory_node->next = node->next;
					node->next = next_memory_node;
				}
			}
			
			//返回申请到的内存地址
			//数组索引为 这一块内存偏移 + 我本身这块内存信息存放的地址大小
			new_memory = (void *)&heap[node->offset + sizeof(memory_node_t)];
			return new_memory;
		}
		
		node = node->next;//如果被使用了，继续找下一块内存
	}
}

int OSfree(void *point)
{
	memory_node_t *node = (memory_node_t *)(point - sizeof(memory_node_t));
	if(node < 0) return 1;   //非法地址
	
	memory_node_t *prev = node->prev;  //上一个内存块地址
	memory_node_t *next = node->next;  //下一个内存块地址
	
	if(prev && prev->used==0 && next && next->used == 0){ //上一块和下一块地址都存在，且都被使用了
		prev->size = prev->size + node->size + next->size + 2 * sizeof(memory_node_t);//将三块内存块大小合并
		prev->next = next->next;
		if(next->next){
			next->next->prev = prev;//让下一块内存块的下一块内存块可以指到合并后的内存块
		}
	}else if(prev && prev->used == 0){//只有前一块内存块可用
		prev->size = prev->size + node->size + sizeof(memory_node_t);
		prev->next = next;
		next->prev = prev;
	}else if(next && next->used == 0){//只有下一块内存块可用
		node->size = next->size + node->size + sizeof(memory_node_t);
		node->next = next->next;
		next->next->prev = node;
		node->used = 0;
	}else{
		node->used = 0;
	}
	return 0;
}


void PrintHearStatus()
{
	memory_node_t *node = memory_all;
	printf("\r\n**************************************************\r\n");
	printf("heap info*****************************************\r\n");
	printf("sizeof memory_node : %d\r\n",sizeof(memory_node_t));
	printf("-----------------------------|\r\n");
	while(node)
	{
		printf("node pointer  : %8X     |\r\n",node);
		printf("node used     : %8d     |\r\n",node->used);
		printf("node size     : %8d     |\r\n",node->size);
		printf("node offset   : %8d     |\r\n",node->offset);
		printf("node next     : %8X     |\r\n",node->next);
	  printf("-----------------------------|\r\n");
		node = node->next;
	}
	
	printf("heao end******************************************\r\n");
	printf("**************************************************\r\n");
}

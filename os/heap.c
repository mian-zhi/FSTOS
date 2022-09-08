#include "heap.h"

//�����ڴ��ڵ�
struct memory_node{
	int size;        //�ڴ���С
	int offset;      //�ڴ��ƫ����
	char used;       //�ڴ���Ƿ�ʹ��
	struct memory_node *prev;   //ָ����һ���ڴ��ĵ�ַ
	struct memory_node *next;   //ָ����һ���ڴ��ĵ�ַ
};

typedef struct memory_node memory_node_t;

char heap[CONFIG_OS_HEAP_SIZE + sizeof(memory_node_t)];//�����ջ

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
	while(node)//�����ڴ��ڵ�
	{
		if(node->used == 0)//�Ƿ�ʹ��
		{
			int size_with_info = size + sizeof(memory_node_t);  //��������Ҫ���ڴ��С
			int remain = node->size - size_with_info;           //������ڴ�鱻ʹ�ú��ʣ��ռ�
			if(remain >= 0){                                     //�������0
				node->size = size;
				node->used = 1;
				if(remain>sizeof(memory_node_t))
				{
					int info_offset = node->offset + size_with_info;  //������һ���ڴ����Զ�����ƫ�Ƶ�ַ
																														//ƫ�Ƶ�ַ = ���ڴ���ƫ�Ƶ�ַ + ��������Ҫ�ĵ�ַ
					memory_node_t *next_memory_node = (memory_node_t *)&heap[info_offset];
																														//�õ���һ���ڴ���ʵ�ʵ�ַ
					
					//��������һ���ڴ�����Ϣ��
					//ԭ���� node ָ����ڴ�飬���������ʣ�µ��ڴ��
					next_memory_node->size = remain - sizeof(memory_node_t);
					next_memory_node->offset = info_offset + sizeof(memory_node_t);  //�ڴ�ƫ�� = �����ڴ��ռ�õ� + �ұ����ڴ�����Ϣ�ڵ�ռ�õ��ڴ�
					next_memory_node->used = 0;
					
					//�����ڴ�����ԭ����������
					next_memory_node->prev = node;
					next_memory_node->next = node->next;
					node->next = next_memory_node;
				}
			}
			
			//�������뵽���ڴ��ַ
			//��������Ϊ ��һ���ڴ�ƫ�� + �ұ�������ڴ���Ϣ��ŵĵ�ַ��С
			new_memory = (void *)&heap[node->offset + sizeof(memory_node_t)];
			return new_memory;
		}
		
		node = node->next;//�����ʹ���ˣ���������һ���ڴ�
	}
}

int OSfree(void *point)
{
	memory_node_t *node = (memory_node_t *)(point - sizeof(memory_node_t));
	if(node < 0) return 1;   //�Ƿ���ַ
	
	memory_node_t *prev = node->prev;  //��һ���ڴ���ַ
	memory_node_t *next = node->next;  //��һ���ڴ���ַ
	
	if(prev && prev->used==0 && next && next->used == 0){ //��һ�����һ���ַ�����ڣ��Ҷ���ʹ����
		prev->size = prev->size + node->size + next->size + 2 * sizeof(memory_node_t);//�������ڴ���С�ϲ�
		prev->next = next->next;
		if(next->next){
			next->next->prev = prev;//����һ���ڴ�����һ���ڴ�����ָ���ϲ�����ڴ��
		}
	}else if(prev && prev->used == 0){//ֻ��ǰһ���ڴ�����
		prev->size = prev->size + node->size + sizeof(memory_node_t);
		prev->next = next;
		next->prev = prev;
	}else if(next && next->used == 0){//ֻ����һ���ڴ�����
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

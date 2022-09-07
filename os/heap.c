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
	
	printf("heap pointer : %x\r\n", heap);
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
			if(remain > 0){                                     //�������0
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
			
			//�������뵽���ڴ��ַ
			//��������Ϊ ��һ���ڴ�ƫ�� + �ұ�������ڴ���Ϣ��ŵĵ�ַ��С
			new_memory = (void *)&heap[node->offset + sizeof(memory_node_t)];
			return new_memory;
		}
		
		node = node->next;//�����ʹ���ˣ���������һ���ڴ�
	}
}



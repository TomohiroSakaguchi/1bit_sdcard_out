#ifndef _SIMPLE_QUEUE_H_
#define _SIMPLE_QUEUE_H_

void queue_init(void);
bool dequeue(uint32_t** buff, uint* cnt);
bool enqueue(uint32_t* buff, uint cnt);
uint32_t get_length(void);

#define QUEUE_SIZE  20          // オーディオバッファの段数

#endif

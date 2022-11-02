#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hardware/sync.h"
#include "simple_queue.h"


#define SPINLOCK_ID_AUDIO_QUEUE (31)

static spin_lock_t* queue_spin_lock;
// 1s = 1000ms
// 3072000sample/s = 3072sample/ms 
static bool audio_buffer[QUEUE_SIZE][3072];          // 1段=1msのサンプルデータ
static uint audio_buffer_len[QUEUE_SIZE];               // QUEUE_SIZE段のバッファの蓄積しているサンプル数

/**
 * バッファ蓄積管理の変数
 */
static uint32_t dequeue_pos;
static uint32_t enqueue_pos;
static uint32_t data_size;

/**
 * バッファ処理の初期化
 */
void queue_init(void)
{
    dequeue_pos = 0;
    enqueue_pos = 0;
    data_size = 0;
    queue_spin_lock = spin_lock_init(SPINLOCK_ID_AUDIO_QUEUE);
}

/**
 * バッファからサンプルを引き取る
 */
bool dequeue(bool** buff, uint32_t* cnt)
{
    bool ret = false;
    if (get_length())
    {
        *cnt = audio_buffer_len[dequeue_pos];
        *buff = audio_buffer[dequeue_pos];
        dequeue_pos = (dequeue_pos + 1) % QUEUE_SIZE;
        uint32_t save = spin_lock_blocking(queue_spin_lock);
        data_size--;
        spin_unlock(queue_spin_lock, save);
        ret = true;
        //printf("dequeue now\n");
    }

    return ret;
}

/**
 * バッファへ値を積む
 */
bool enqueue(bool* buff, uint32_t cnt)
{
    bool ret = false;
    if (get_length() < QUEUE_SIZE){
        //printf("enqueue now\n");
        for(uint i=0; i<cnt*1; i++)                           // buffの内容をenqueueバッファにコピー
        {
           audio_buffer[enqueue_pos][i] = (bool)(buff[i]);
           //printf("audio_buffer[%d]=%d\n",i,buff[i]);
        }
        audio_buffer_len[enqueue_pos] = cnt;
        //printf("audio_buffer_len[%d] = %d\n",enqueue_pos,audio_buffer_len[enqueue_pos]);
        enqueue_pos = (enqueue_pos + 1) % QUEUE_SIZE;
        uint32_t save = spin_lock_blocking(queue_spin_lock);
        data_size++;
        spin_unlock(queue_spin_lock, save);
        ret = true;
    }
    return ret;
}

/**
 * バッファの値の蓄積量の取得
 */
uint32_t get_length(void)
{
    uint32_t ret;
    uint32_t save = spin_lock_blocking(queue_spin_lock);
    ret = data_size;
    spin_unlock(queue_spin_lock, save);
    //printf("data_size = %d\n",ret);
    return ret;
}
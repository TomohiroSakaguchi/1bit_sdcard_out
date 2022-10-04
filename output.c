#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "simple_queue.h"
#include "pico/multicore.h"
#include "main.h"

#define FS              48000           // Source Sampling Frequency[Hz]
#define OSR             64              // Over Sampling Rate 
#define OSR_BSFT        6               // BitShift equivalent of Over Sampling Rate , log2(OSR)
#define DS_DC_OFFSET    (0x07ff << OSR_BSFT)    // Delta-Sigma DC offset for idle tone reduction
#define OFS             (FS * OSR)      // Output/Over Sampling Frequency[Hz]
#define N_CH            1               // Number of Audio Channel
#define OS_ORDER        2               // OverSampler次数 0:0次ホールド 1:直線補間 2:二次曲線補間
#define DE_SINC3        1               // OverSampler補正(逆SINC3)フィルタ 0:無効 1:有効
#define FIR_T_MAX       15              // FIRフィルタ構造体用最大タップ数定義

void output(){
	static bool mute_flag = false;
    static uint32_t mute_buff[768] = {0};  // 無音buff 3072/4でよさそう？
    uint count;
    uint32_t* buff; //buffのポインタを宣言するのでbuffそのものを呼び出しているわけではない
    
    while(1){
        printf("Now is in while\n");
    	uint32_t length = get_length();
    	
    	if((length == 0)&&(mute_flag ==false))              // mute開始条件段数
        {
            printf("flag is mute\n");
            mute_flag = true;
        } 
        else if(length >= 5)            // mute解除条件段数
        {
            printf("flag is on\n");
            mute_flag = false;
        }

        if(mute_flag||(dequeue(&buff, &count) == false))    // mute状態もしくはdequeue失敗ならmute_bufferに切り替え
        {
            printf("flag is mute due to fail dequeue\n");
            buff = mute_buff;
            count = sizeof(mute_buff) / (sizeof(int32_t) * 2);
        }
        printf("%d",length);

        uint32_t bs;
        for(uint32_t i = 0; i<count; i=i+32){
        	//32bitをかたまりにできればいい
        	bs = 0;
        	for(uint32_t j=0; j<32; j++){
        	    printf("buff[%d] = %d\n" , i+j,buff[i+j]);
        	    bs = bs + buff[i+j];
        	}
        	printf("bs_No.%d = %d\n",(i/32),bs);
        }
        //sem_release(&buffout_initted);
    }
}
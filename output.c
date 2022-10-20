#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "simple_queue.h"
#include "pico/multicore.h"
#include "main.h"
#include "output.pio.h"
#include "pins.h"

#define FS              48000           // Source Sampling Frequency[Hz]
#define OSR             64              // Over Sampling Rate 
#define OSR_BSFT        6               // BitShift equivalent of Over Sampling Rate , log2(OSR)
#define DS_DC_OFFSET    (0x07ff << OSR_BSFT)    // Delta-Sigma DC offset for idle tone reduction
#define OFS             (FS * OSR)      // Output/Over Sampling Frequency[Hz]
//#define OFS             5644800      // Output/Over Sampling Frequency[Hz]
#define N_CH            1               // Number of Audio Channel
#define OS_ORDER        2               // OverSampler次数 0:0次ホールド 1:直線補間 2:二次曲線補間
#define DE_SINC3        1               // OverSampler補正(逆SINC3)フィルタ 0:無効 1:有効
#define FIR_T_MAX       15              // FIRフィルタ構造体用最大タップ数定義

// Test DipSW Init. & DipSW Reader

uint32_t get_dip(void){
    static bool first = 1;
    if (first){
        first = false;
        gpio_init_mask(PIN_DIP_MASK);
        gpio_set_dir_in_masked(PIN_DIP_MASK);
        for(uint32_t i = 0; i < PIN_DIP_BIT_WIDTH; i++)
            gpio_pull_up(PIN_DIP_0 +i);
    }
    return (gpio_get_all() & PIN_DIP_MASK) >> PIN_DIP_0;
}

// GPIO Pad(DriveStrength,PU,PD,SlewFast) Setting Functions
void set_gpio_pad_drive(uint32_t gpio_num, uint32_t set_data){
    gpio_num &= 31; // Limited to 0-31
    set_data &= 3;  // Limited to 0-3 
    uint32_t *adr =(uint32_t *)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + gpio_num * 4);
    uint32_t data = (uint32_t)*adr;
    data &= ~PADS_BANK0_GPIO0_DRIVE_BITS; // Target bit clear
    data |= set_data << PADS_BANK0_GPIO0_DRIVE_LSB;
    *adr = data;
}

void set_gpio_pad_pue(uint32_t gpio_num, uint32_t set_data){
    gpio_num &= 31; // Limited to 0-31
    set_data &= 1;  // Limited to 0-1 
    uint32_t *adr =(uint32_t *)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + gpio_num * 4);
    uint32_t data = (uint32_t)*adr;
    data &= ~PADS_BANK0_GPIO0_PUE_BITS; // Target bit clear
    data |= set_data << PADS_BANK0_GPIO0_PUE_LSB;
    *adr = data;
}

void set_gpio_pad_pde(uint32_t gpio_num, uint32_t set_data){
    gpio_num &= 31; // Limited to 0-31
    set_data &= 1;  // Limited to 0-1 
    uint32_t *adr =(uint32_t *)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + gpio_num * 4);
    uint32_t data = (uint32_t)*adr;
    data &= ~PADS_BANK0_GPIO0_PDE_BITS; // Target bit clear
    data |= set_data << PADS_BANK0_GPIO0_PDE_LSB;
    *adr = data;
}

void set_gpio_pad_slewfast(uint32_t gpio_num, uint32_t set_data){
    gpio_num &= 31; // Limited to 0-31
    set_data &= 1;  // Limited to 0-1 
    uint32_t *adr =(uint32_t *)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + gpio_num * 4);
    uint32_t data = (uint32_t)*adr;
    data &= ~PADS_BANK0_GPIO0_SLEWFAST_BITS; // Target bit clear
    data |= set_data << PADS_BANK0_GPIO0_SLEWFAST_LSB;
    *adr = data;
}

void output(){
    uint offset = pio_add_program(pio0, &pio_pdm_output_program);
    pio_pdm_output_program_init(pio0, 0, offset, OFS, PIN_OUTPUT_LP);   // Init. Left Ch. pio，pdm_output.pioで定義
    pio_pdm_output_program_init(pio0, 1, offset, OFS, PIN_OUTPUT_RP);   // Init. Right Ch. pio，pdm_output.pioで定義
    pio_sm_set_enabled(pio0, 0, true);                                  // Start Left Ch. pio
    pio_sm_set_enabled(pio0, 1, true);                                  // Start Right Ch. pio

    // PDM GPIO Pad Setting
    for(uint32_t i = PIN_OUTPUT_RP; i <= PIN_OUTPUT_LN; i++){
        set_gpio_pad_drive(i, 3);       // DriveStrength  0,1,2,3:2,4,8,12mA
        set_gpio_pad_pue(i, 0);         // PullUpEnable   0:Disable,1:Enable
        set_gpio_pad_pde(i, 0);         // PullDownEnable 0:Disable,1:Enable
        set_gpio_pad_slewfast(i, 1);    // SlewFast       0:Disable,1:Enable
    }

	static bool mute_flag = false;
    static bool mute_buff[768] = {false};  // 無音buff 3072/4でよさそう？
    uint32_t count;
    bool* buff; //buffのポインタを宣言するのでbuffそのものを呼び出しているわけではない
    
    while(1){
        //printf("Now is in while\n");
    	uint32_t length = get_length();
    	
    	if((length < 5)&&(mute_flag ==false))              // mute開始条件段数
        {
            printf("flag is mute\n");
            mute_flag = true;
        } 
        else if(length >= 15)            // mute解除条件段数
        {
            printf("flag is on\n");
            mute_flag = false;
            printf("mute flag = %d\n",mute_flag);
        }
        printf("check point1\n");
        if(mute_flag||(dequeue(&buff, &count) == false))    // mute状態もしくはdequeue失敗ならmute_bufferに切り替え
        
        {
            printf("flag is mute due to fail dequeue\n");
            buff = mute_buff;
            count = sizeof(mute_buff);
        }
        //printf("%d",length);

        uint32_t bs;
        uint32_t buff_ct = 0;
        printf("count = %d\n",count);
        for(uint32_t i = 0; i<count; i++){
            bool d0 = buff[buff_ct++];
            //printf("d0 = %d\n",d0);
        	if(d0 == true){bs = 0x7fffffff;}
            else {bs = 0x00000000;}

        	//printf("bs[%d] = %d\n",i,bs);
//            gpio_put(PIN_PIOT_MEASURE, 1);              // テスト用 pio設定前にH。pioに待たされている時刻測定用
            //sem_release(&sem);
            pio_sm_put_blocking(pio0, 0, bs);  //　set L-Ch 31:00 data to pio0,sm0(LSB First)
            //pio_sm_put_blocking(pio0, 1, bs);  //　set R-Ch 31:00 data to pio0,sm1(LSB First) 
            //pio_sm_put_blocking(pio0, 0, ch[0].bs[1]);  //　set L-Ch 63:32 data to pio0,sm0
            //pio_sm_put_blocking(pio0, 1, ch[1].bs[1]);  //　set R-Ch 63:32 data to pio0,sm1

//          gpio_put(PIN_PIOT_MEASURE, 0);              // テスト用 pio設定後にL。pioに待たされている時刻測定用
        }
        //sem_release(&buffout_initted);
    }
}
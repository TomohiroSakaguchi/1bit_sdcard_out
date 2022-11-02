#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "simple_queue.h"
#include "pico/multicore.h"
#include "sd_card.h"
#include "ff.h"
#include "output.h"
//#include "hardware/pio.h"
//#include "hardware/sync.h"
//#include "hardware/pll.h"
#include "output.pio.h"
#include "pins.h"

//static semaphore_t sem;

char parseint(char str){
	return str - '0';
}

void file_read() {

    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[3072];
    char *ptr;
    char filename[] = "check_500Hz_3072000Fs_10s.dat";
    //char filename[] = "check_500Hz_3072000Fs_10s.dat";

    // Wait for user to press 'enter' to continue
    printf("\r\nSD card test. Press 'enter' to start.\r\n");
    while (true) {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n')) {
            break;
        }
    }

    // Initialize SD card
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    // Mount drive
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }
/*
    // Open file for writing ()
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Write something to file
    ret = f_printf(&fil, "This is another test\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }
    ret = f_printf(&fil, "of writing to an SD card.\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }
*/
    // Open file for reading
    /* Choose read file
    printf("Input you want to read filename\r\n");
    gets(filename);
    if (!filename)
    {
        char filename[] = "check_500Hz_3072000Fs.dat";
    }
    */
    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Print every line in file over serial
    printf("Reading from file '%s':\r\n", filename);
    printf("---\r\n");
    while(1){
        /*if(get_length() > 15){
            //sem_acquire_blocking(&sem);
            //multicore_launch_core1(output);
            break;
        }*/
        while (f_gets(buf, sizeof(buf), &fil)) {
            uint32_t data[sizeof(buf)];
            bool data_bool[sizeof(buf)];
            //uint32_t *data;
            for(uint32_t i=0;i<sizeof(buf);i++){
                data[i] = parseint(buf[i]);
                if(data[i] == 1 ){ data_bool[i] = true;}
                else { data_bool[i] = false;}
                //printf("data[%d] = %d\n",i,data_bool[i]);
            }
            //printf("%d",data);
            enqueue(data_bool,sizeof(buf));
            //printf("length = %d\n",get_length());
            //printf("start multicore\n");
            //multicore_launch_core1(output);
            //break;
            }
    }
    printf("\r\n---\r\n");

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // Unmount drive
    f_unmount("0:");

    /*
    // Loop forever doing nothing
    while (true) {
        sleep_ms(1000);
    }*/
}

int main(){
     //  PDM動作に最適なCPU周波数の設定
//////////////////////////////////////////  Met S/N 
//  set_sys_clock_khz(133000, true);    //  NG  -   Core1 overflow
//  set_sys_clock_khz(140000, true);    //  NG  -   Core1 overflow
//  set_sys_clock_khz(144000, true);    //  OK  NG  144.0M/48k/64 = 46.875 -> Low S/N
//  set_sys_clock_khz(150000, true);    //  OK  NG  150.0M/48k/64 = 48.828 -> Low S/N  
//    set_sys_clock_khz(153600, true);    //  NG    153.6M/48k/64 = 50.000, 153.6M/133M = x1.15 Overclock ->手持ちのPicoではUSBデバイスと新式されず．．
    set_sys_clock_khz(230400, true);    //  OK  OK  230.4M/48k/64 = 75.000, 230.4M/133M = x1.73 Overclock
    //stdio_uart_init();
    // Initialize chosen serial port
    stdio_init_all();
    queue_init();
    //sem_init(&sem, 1, 1);
    //printf("begin file_read from main\n");
    file_read();
    //printf("start multicore1\n");
    //sem_release(&sem);
    multicore_launch_core1(output);

}
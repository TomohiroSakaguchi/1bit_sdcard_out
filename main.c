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

static semaphore_t buffout_initted;

char parseint(char str){
	return str - '0';
}

void file_read() {

    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char *ptr;
    char filename[] = "check_500Hz_3072000Fs.dat";

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
    while (f_gets(buf, sizeof(buf), &fil)) {
        uint32_t data[sizeof(buf)];
        for(uint32_t i=0;i<sizeof(buf);i++){
            data[i] = parseint(buf[i]);
            //printf("%d",data[i]);
        }
        //printf("%d",data);
        enqueue(data,sizeof(buf));
        printf("start multicore1\n");
        multicore_launch_core1(output);
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
    set_sys_clock_khz(153600, true);    //  OK  OK  153.6M/48k/64 = 50.000, 153.6M/133M = x1.15 Overclock
//  set_sys_clock_khz(230400, true);    //  OK  OK  230.4M/48k/64 = 75.000, 230.4M/133M = x1.73 Overclock
    //stdio_uart_init();
    // Initialize chosen serial port
    stdio_init_all();
    queue_init();
    //sem_init(&buffout_initted, 0, 1);
    //sem_acquire_blocking(&buffout_initted);
    file_read();
    //printf("start multicore1\n");
    //multicore_launch_core1(output);
}
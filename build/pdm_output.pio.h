// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// -------------- //
// pio_pdm_output //
// -------------- //

#define pio_pdm_output_wrap_target 0
#define pio_pdm_output_wrap 16

#define pio_pdm_output_offset_start_output 0u

static const uint16_t pio_pdm_output_program_instructions[] = {
            //     .wrap_target
    0xe049, //  0: set    y, 9                       
    0xe026, //  1: set    x, 6                       
    0x4044, //  2: in     y, 4                       
    0x4024, //  3: in     x, 4                       
    0x40c8, //  4: in     isr, 8                     
    0x40d0, //  5: in     isr, 16                    
    0xa242, //  6: nop                           [2] 
    0x00ea, //  7: jmp    !osre, 10                  
    0xa0e6, //  8: mov    osr, isr                   
    0x000b, //  9: jmp    11                         
    0xa142, // 10: nop                           [1] 
    0x6041, // 11: out    y, 1                       
    0x006f, // 12: jmp    !y, 15                     
    0xe002, // 13: set    pins, 2                    
    0x0006, // 14: jmp    6                          
    0xe001, // 15: set    pins, 1                    
    0x0006, // 16: jmp    6                          
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program pio_pdm_output_program = {
    .instructions = pio_pdm_output_program_instructions,
    .length = 17,
    .origin = -1,
};

static inline pio_sm_config pio_pdm_output_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + pio_pdm_output_wrap_target, offset + pio_pdm_output_wrap);
    return c;
}

#include "hardware/clocks.h"
static inline void pio_pdm_output_program_init(PIO pio, uint sm, uint offset, uint baud, uint pin_p)
{
    pio_gpio_init(pio, pin_p);                              // GPIOn   for positive pin
    pio_gpio_init(pio, pin_p + 1);                          // GPIOn+1 for negative pin
    pio_sm_set_consecutive_pindirs(pio, sm, pin_p, 2, true);// pin_base = pin_p, pin_count = 2, output 
    pio_sm_config c = pio_pdm_output_program_get_default_config(offset);
//  sm_config_set_out_pins(&c, pin_p, 2);                   // for 'out' pins command, NOT USED in pio_pdm_output program.
    sm_config_set_set_pins(&c, pin_p, 2);                   // for 'set' pins command.
    sm_config_set_out_shift(&c, true, true, 32);            // osr : shift right, autopull, threshold = 32 
    sm_config_set_in_shift(&c, false, false, 32);           // isr : shift left, no autopush, threshold = 32
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);          // Deeper FIFO as we're not doing any RX
    float div = (float)clock_get_hz(clk_sys) / (10 * baud); // We transmit 1 bit every 10 execution cycles
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio, sm, offset, &c);
//  pio_sm_set_enabled(pio, sm, true);                      // Move to higher-level program for L/R start phase-alignment.
}

#endif

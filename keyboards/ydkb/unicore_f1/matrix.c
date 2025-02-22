/*
Copyright 2022 YANG <drk@live.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* rough code */

#include "ch.h"
#include "hal.h"

/*
 * scan matrix
 */
#include "action.h"
#include "print.h"
#include "debug.h"
#include "timer.h"
#include "util.h"
#include "matrix.h"
#include "debounce_pk.h"
#include "wait.h"
#include "switch_board.h"
#include "rgblight.h"

#undef DOUBLE_CLICK_FIX_DELAY
#define DOUBLE_CLICK_FIX_DELAY 15

extern debug_config_t debug_config;

static matrix_row_t matrix[MATRIX_ROWS] = {0};
static uint16_t matrix_scan_timestamp = 0;
static uint8_t matrix_debouncing[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint8_t matrix_double_click_fix[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint8_t now_debounce_dn_mask = DEBOUNCE_NK_MASK;
static bool matrix_idle = false;
static bool first_key_scan = false;

static void select_key(uint8_t mode);
static void select_all_keys(void);
static uint8_t get_key(void);
static void init_cols(void);
__attribute__ ((weak))
void matrix_scan_user(void) {}

__attribute__ ((weak))
void matrix_scan_kb(void)
{
    matrix_scan_user();
    hook_keyboard_loop();
}

void matrix_init(void)
{
    debug_config.enable = 1;
    debug_config.matrix = 0;

    init_cols();
}

static bool process_key_press = 0;
bool should_process_keypress(void) {
    return process_key_press;
}

uint8_t matrix_scan(void)
{
    matrix_scan_quantum(); // use this to run hook_keyboard_loop()

    if (matrix_idle) {
        if (get_key() == 0) return 1;
        else {
            matrix_idle = false;
            first_key_scan = true;
        }
    }
    if (!first_key_scan) {
        //scan matrix every 1ms
        uint16_t time_check = timer_read();
        if (matrix_scan_timestamp == time_check) return 1;
        matrix_scan_timestamp = time_check;
    }

    select_key(0);
    uint8_t matrix_keys_idle = 0;
    for (uint8_t row=0; row<MATRIX_ROWS; row++) {
        for (uint8_t col=0; col<MATRIX_COLS; col++) {
            uint8_t *debounce = &matrix_debouncing[row][col];
            uint8_t *double_click_fix = &matrix_double_click_fix[row][col];

            uint8_t key = get_key();
            *debounce = (*debounce >> 1) | key;
            //select next key
            select_key(1);
            if (1) {
                matrix_row_t *p_row = &matrix[row];
                matrix_row_t col_mask = ((matrix_row_t)1 << col);
                if (*double_click_fix > 0 && (*p_row & col_mask) == 0) {
                    (*double_click_fix)--;
                } else {
                    if        (*debounce > now_debounce_dn_mask) {  //debounce KEY DOWN 
                        *p_row |=  col_mask;
                        *double_click_fix = DOUBLE_CLICK_FIX_DELAY; 
                    } else if (*debounce < DEBOUNCE_UP_MASK) { //debounce KEY UP
                        *p_row &= ~col_mask;
                        matrix_keys_idle++;
                    }
                }
            }
        }
    }

    // to avoid all the keys being down in some cases like KEY is connected to GND.
    process_key_press = (matrix_keys_idle > 0);

    // no key down, set matrix_idle.
    if (matrix_keys_idle == MATRIX_ROWS * MATRIX_COLS) {
        select_all_keys();
        matrix_idle = true;
    } else {
        if (first_key_scan) {
            first_key_scan = false;
            #if DEBOUNCE_NK > 0
            matrix_scan();
            #endif
        }
    }

    return 1;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{

}

uint8_t matrix_key_count(void)
{
    return 0;
}

static void init_cols(void)
{
    //595 pin
    palSetGroupMode(GPIOB,  (1<<13 | 1<<12), 0 , PAL_MODE_OUTPUT_PUSHPULL);
}

 
static uint8_t get_key(void)
{
    return (palReadPad(GPIOB, 13)==PAL_HIGH) ? 0 : 0x80;
}

void select_all_keys(void)
{
    select_key_ready();

    DS_PL_LO();
    for (uint8_t i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        CLOCK_PULSE();
    }
    get_key_ready();
}


static void select_key(uint8_t mode)
{
    select_key_ready();
    if (mode == 0) {
        DS_PL_HI();
        for (uint8_t i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
            CLOCK_PULSE();
        }
        DS_PL_LO();
        CLOCK_PULSE();
    } else {
        DS_PL_HI();
        CLOCK_PULSE();
    }
    get_key_ready();
}

void bootmagic_lite(void)
{
#ifdef SOFTWARE_ESC_BOOTLOADER
    wait_ms(200);
    matrix_scan();
    matrix_scan();
    // only the first key(esc) is pressed
    uint16_t boot_key = matrix_get_row(0);
    if (boot_key == 1) {  // only top left
        uint8_t row = MATRIX_ROWS;
        while (row-- > 1) boot_key += matrix_get_row(row);
        if (boot_key == 1) enter_bootloader();
    }
#endif
}

void early_hardware_init_pre(void)
{
    // Override hard-wired USB pullup to disconnect and reconnect
    palSetPadMode(GPIOA, 12, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOA, 12);
    for (uint32_t i = 0; i < 800000; i++) {
        __asm__("nop");
    }
}

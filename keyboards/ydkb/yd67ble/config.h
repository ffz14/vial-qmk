#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define FW_VER          QMK_DMC8
#define FW_VER_VIA      VIA_DMC8
#define VENDOR_ID       0x9D5B 
#define PRODUCT_ID      0x2167  
#define DEVICE_VER      0x0011
#define MANUFACTURER    YDKB
#define PRODUCT         YD67BLE (FW_VER)


#define MATRIX_ROWS 5  //595 num of each side.
#define MATRIX_COLS 16



#define TAPPING_TOGGLE  2

#define TAPPING_TERM    200
#define IGNORE_MOD_TAP_INTERRUPT // this makes it possible to do rolling combos (zx) with keys that convert to other keys on hold (z becomes ctrl when you hold it, and when this option isn't enabled, z rapidly followed by x actually sends Ctrl-x. That's bad.)


/* key combination for command */
#define IS_COMMAND() ( \
    (get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT))) || \
    (get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_LCTRL) | MOD_BIT(KC_RSHIFT))) \
)
/* disable command for default layer */
#define MAGIC_KEY_SWITCH_LAYER_WITH_FKEYS  0
#define MAGIC_KEY_SWITCH_LAYER_WITH_NKEYS  0

/* fix space cadet rollover issue */
#define DISABLE_SPACE_CADET_ROLLOVER

#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB1286__)
    #define UCSR1D _SFR_MEM8(0xCB)
    #define RTSEN 0
    #define CTSEN 1

    #define SERIAL_UART_BAUD        76800
    #define SERIAL_UART_DATA        UDR1
    #define SERIAL_UART_UBRR        ((F_CPU/(8.0*SERIAL_UART_BAUD)-1+0.5))
    #define SERIAL_UART_RXD_VECT    USART1_RX_vect
    #define SERIAL_UART_TXD_READY   (UCSR1A&(1<<UDRE1))
    #define SERIAL_UART_INIT()      do { \
        cli(); \
        UBRR1L = (uint8_t) SERIAL_UART_UBRR;      \
        UBRR1H = ((uint16_t)SERIAL_UART_UBRR>>8); \
        UCSR1B |= (1<<RXCIE1) | (1<<RXEN1); \
        UCSR1B |= (0<<TXCIE1) | (1<<TXEN1); \
        UCSR1C |= (0<<UPM11) | (0<<UPM10); \
        UCSR1A |= (1<<U2X1); \
        sei(); \
    } while(0)
#else
    #error "USART configuration is needed."
#endif
/* BT Power Control */
#define BT_POWERED    (~PORTF & (1<<7))
#define bt_power_init()    do { DDRF |= (1<<7); PORTF &= ~(1<<7);} while(0)
#define turn_off_bt()    do { PORTF |= (1<<7); UCSR1B &= ~(1<<TXEN1); } while(0)
#define turn_on_bt()    do { PORTF &= ~(1<<7); UCSR1B |= (1<<TXEN1);} while(0)

#define BLE_NAME "YD67BLE"
#define BLE_BATTERY_SERVICE
#define BT_POWER_SAVE_TIME 3000

#define UPDATE_BATTERY_WHEN_CHARGING
#define BATTERY_CHARGING (~PINC & (1<<7))
#define CHARGING_FIX_VALUE 90
#define CHARGING_STATE_INIT()    do { DDRC &= ~(1<<7); PORTC |= (1<<7);} while(0)
/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */
/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT
//#define NO_ACTION_MACRO
//#define NO_ACTION_FUNCTION

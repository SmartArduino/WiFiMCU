/**
******************************************************************************
* @file    platform_mcu_powersave.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide functions called by MICO to drive stm32f2xx 
*          platform: - e.g. power save, reboot, platform initialize
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 

#include "platform.h"
#include "platform_peripheral.h"

#include "platform_config.h"
#include "platform_init.h"
#include "MiCORTOS.h"

/******************************************************
*                      Macros
******************************************************/

#define CYCLES_TO_MS( cycles ) ( ( ( cycles ) * 1000 * RTT_CLOCK_PRESCALER ) / RTT_CLOCK_HZ )
#define MS_TO_CYCLES( ms )     ( ( ( ms ) * RTT_CLOCK_HZ ) / ( 1000 * RTT_CLOCK_PRESCALER ) )

#ifndef ENABLE_INTERRUPTS
#define ENABLE_INTERRUPTS   __asm("CPSIE i")  
#endif

#ifndef DISABLE_INTERRUPTS
#define DISABLE_INTERRUPTS  __asm("CPSID i")  /**< Disable interrupts to stop task switching in MICO RTOS. */
#endif

/******************************************************
*                    Constants
******************************************************/

#define RTT_CLOCK_PRESCALER          (4) /*                                                                                        */
#define RTT_CLOCK_HZ             (32768) /* 32K / prescaler                                                                        */
#define RTT_MAX_CYCLES           (64000) /* 7.8125 seconds                                                                         */
#define RC_OSC_DELAY_CYCLES         (15) /* Cycles required to stabilise clock after exiting WAIT mode                             */
#define JTAG_DEBUG_SLEEP_DELAY_MS (3000) /* Delay in ms to give OpenOCD enough time to halt the CPU before the CPU enter WAIT mode */

#define WAIT_MODE_SUPPORT
#define WAIT_MODE_ENTER_DELAY_CYCLES  (3) 
#define WAIT_MODE_EXIT_DELAY_CYCLES   (34)

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/
#ifndef MICO_DISABLE_MCU_POWERSAVE
static unsigned long  wait_mode_power_down_hook( unsigned long sleep_ms );
#else
static unsigned long  idle_power_down_hook( unsigned long sleep_ms );
#endif

/******************************************************
*               Variables Definitions
******************************************************/

#ifndef MICO_DISABLE_MCU_POWERSAVE
static volatile uint32_t     system_io_backup_value      = 0;
static volatile uint32_t     samg5x_clock_needed_counter  = 0;
static volatile bool         wake_up_interrupt_triggered = false;
#endif /* MICO_DISABLE_MCU_POWERSAVE */

/******************************************************
 *               Function Definitions
 ******************************************************/

OSStatus platform_mcu_powersave_init(void)
{
#ifndef MICO_DISABLE_MCU_POWERSAVE
    #error Not working currently, uncomment MICO_DISABLE_MCU_POWERSAVE in platform_config.h
    /* Initialise all pins to be input pull-up to save power */
    ioport_enable_port( IOPORT_PIOA,   0xffffffffU );
    ioport_set_port_mode( IOPORT_PIOA, 0xffffffffU, IOPORT_MODE_PULLUP );
    ioport_set_port_dir( IOPORT_PIOA,  0xffffffffU, IOPORT_DIR_INPUT );

    ioport_enable_port( IOPORT_PIOB,   0xffffffffU );
    ioport_set_port_mode( IOPORT_PIOB, 0xffffffffU, IOPORT_MODE_PULLUP );
    ioport_set_port_dir( IOPORT_PIOB,  0xffffffffU, IOPORT_DIR_INPUT );

    NVIC_DisableIRQ( RTT_IRQn );
    NVIC_ClearPendingIRQ( RTT_IRQn );
    NVIC_EnableIRQ( RTT_IRQn );
    pmc_set_fast_startup_input( PMC_FSMR_RTTAL );


    rtt_init( RTT, RTT_CLOCK_PRESCALER );
    rtt_write_alarm_time( RTT, 64000 );
   

#endif /* MICO_DISABLE_MCU_POWERSAVE */

    return kNoErr;
}

OSStatus platform_mcu_powersave_disable( void )
{
#ifndef MICO_DISABLE_MCU_POWERSAVE

    /* Atomic operation starts */
    DISABLE_INTERRUPTS;

    /* Increment counter to indicate CPU clock is needed preventing CPU from entering WAIT mode */
    samg5x_clock_needed_counter++;

    /* Atomic operation ends */
    ENABLE_INTERRUPTS;

#endif /* MICO_DISABLE_MCU_POWERSAVE */
    return kNoErr;
}

OSStatus platform_mcu_powersave_enable( void )
{
#ifndef MICO_DISABLE_MCU_POWERSAVE
    /* Atomic operation starts */
    DISABLE_INTERRUPTS;

    /* Decrement counter only if it's not 0 */
    samg5x_clock_needed_counter -= ( samg5x_clock_needed_counter > 0 ) ? 1 : 0;

    /* Atomic operation ends */
    ENABLE_INTERRUPTS;
#endif /* MICO_DISABLE_MCU_POWERSAVE */
    return kNoErr;
}

OSStatus platform_powersave_enable_wakeup_pin( const platform_gpio_t* gpio )
{
#ifndef MICO_DISABLE_MCU_POWERSAVE
    if ( gpio->is_wakeup_pin == true )
    {
        /* Enable wake-up pin */
        pmc_set_fast_startup_input( 1 << gpio->wakeup_pin_number );

        if ( gpio->trigger == IOPORT_SENSE_RISING )
        {
            /* Set polarity of corresponding wake-up pin bit to active-high */
            PMC->PMC_FSPR |=  (uint32_t)( 1 << gpio->wakeup_pin_number );
        }
        else
        {
            /* Set polarity of corresponding wake-up pin bit to active-low */
            PMC->PMC_FSPR &= ~(uint32_t)( 1 << gpio->wakeup_pin_number );
        }

        return kNoErr;
    }
    else
    {
        return kGeneralErr;
    }
#else /* MICO_DISABLE_MCU_POWERSAVE */
    return kNoErr;
#endif
}

OSStatus platform_powersave_disable_wakeup_pin( const platform_gpio_t* gpio )
{
#ifndef MICO_DISABLE_MCU_POWERSAVE
    if ( gpio->is_wakeup_pin == true )
    {
        pmc_clr_fast_startup_input( 1 << gpio->wakeup_pin_number );
        return kNoErr;
    }
    else
    {
        return kGeneralErr;
    }
#else /* MICO_DISABLE_MCU_POWERSAVE */
    return kNoErr;
#endif
}

void platform_mcu_powersave_exit_notify( void )
{
#ifndef MICO_DISABLE_MCU_POWERSAVE
    wake_up_interrupt_triggered = true;
#endif /* MICO_DISABLE_MCU_POWERSAVE */
}

/******************************************************
 *               RTOS Powersave Hooks
 ******************************************************/

void platform_idle_hook( void )
{
    __asm("wfi");
}

uint32_t platform_power_down_hook( uint32_t sleep_ms )
{
#ifdef MICO_DISABLE_MCU_POWERSAVE
    /* If MCU powersave feature is disabled, enter idle mode when powerdown hook is called by the RTOS */
    return idle_power_down_hook( sleep_ms );

#else
    /* If MCU powersave feature is enabled, enter STOP mode when powerdown hook is called by the RTOS */
    return wait_mode_power_down_hook( sleep_ms );

#endif
}


/******************************************************
 *            Static Function Definitions
 ******************************************************/

#ifdef MICO_DISABLE_MCU_POWERSAVE
/* MCU Powersave is disabled */
static unsigned long idle_power_down_hook( unsigned long sleep_ms  )
{
    UNUSED_PARAMETER( sleep_ms );
    ENABLE_INTERRUPTS;;
    __asm("wfi");
    return 0;
}
#else
static unsigned long wait_mode_power_down_hook( unsigned long delay_ms )
{
    bool jtag_enabled       = ( ( CoreDebug ->DHCSR & CoreDebug_DEMCR_TRCENA_Msk ) != 0 ) ? true : false;
    bool jtag_delay_elapsed = ( mico_get_time() > JTAG_DEBUG_SLEEP_DELAY_MS ) ? true : false;
    uint32_t     elapsed_cycles     = 0;

    /* Criteria to enter WAIT mode
     * 1. Clock needed counter is 0 and no JTAG debugging
     * 2. Clock needed counter is 0, in JTAG debugging session, and WICED system tick has progressed over 3 seconds.
     *    This is to give OpenOCD enough time to poke the JTAG tap before the CPU enters WAIT mode.
     */
    if ( ( samg5x_clock_needed_counter == 0 ) && ( ( jtag_enabled == false ) || ( ( jtag_enabled == true ) && ( jtag_delay_elapsed == true ) ) ) )
    {
        uint32_t total_sleep_cycles;
        uint32_t total_delay_cycles;

        /* Start real-time timer */
        rtt_init( RTT, RTT_CLOCK_PRESCALER );

        /* Start atomic operation */
        DISABLE_INTERRUPTS;

        /* Ensure deep sleep bit is enabled, otherwise system doesn't go into deep sleep */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Disable SysTick */
        SysTick->CTRL &= ( ~( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk ) );

        /* End atomic operation */
        ENABLE_INTERRUPTS;

        /* Expected total time CPU executing in this function (including WAIT mode time) */
        total_sleep_cycles = MS_TO_CYCLES( delay_ms );

        /* Total cycles in WAIT mode loop */
        total_delay_cycles = ( total_sleep_cycles / RTT_MAX_CYCLES + 1 ) * RC_OSC_DELAY_CYCLES + WAIT_MODE_ENTER_DELAY_CYCLES + WAIT_MODE_EXIT_DELAY_CYCLES;

        if ( total_sleep_cycles > total_delay_cycles )
        {
            /* Adjust total sleep cycle to exclude exit delay */
            total_sleep_cycles -= WAIT_MODE_EXIT_DELAY_CYCLES;

            /* Prepare platform specific settings before entering powersave */
//            platform_enter_powersave();

            ///* Prepare WLAN bus before entering powersave */
            //platform_bus_enter_powersave();

            /* Disable brownout detector */
            supc_disable_brownout_detector( SUPC );

            /* Backup system I/0 functions and set all to GPIO to save power */
            system_io_backup_value = matrix_get_system_io();
            matrix_set_system_io( 0x0CF0 );

            /* Switch Master Clock to Main Clock (internal fast RC oscillator) */
            pmc_switch_mck_to_mainck( PMC_PCK_PRES_CLK_1 );

            /* Switch on internal fast RC oscillator, switch Main Clock source to internal fast RC oscillator and disables external fast crystal */
            pmc_switch_mainck_to_fastrc( CKGR_MOR_MOSCRCF_8_MHz );

            /* Disable external fast crystal */
            pmc_osc_disable_xtal( 0 );

            /* Disable PLLA */
            pmc_disable_pllack( );

            /* This above process introduces certain delay. Add delay to the elapsed cycles */
            elapsed_cycles += rtt_read_timer_value( RTT );

            while ( wake_up_interrupt_triggered == false  && elapsed_cycles < total_sleep_cycles )
            {
                uint32_t current_sleep_cycles = total_sleep_cycles - elapsed_cycles;

                /* Start real-time timer and alarm */
                rtt_init( RTT, RTT_CLOCK_PRESCALER );
                rtt_write_alarm_time( RTT, ( current_sleep_cycles > RTT_MAX_CYCLES ) ? RTT_MAX_CYCLES - RC_OSC_DELAY_CYCLES : current_sleep_cycles - RC_OSC_DELAY_CYCLES );

                __asm("wfi");
                /* Enter WAIT mode */
                //pmc_enable_waitmode();

                /* Clear wake-up status */
                rtt_get_status( RTT );

                /* Add sleep time to the elapsed cycles */
                elapsed_cycles += rtt_read_timer_value( RTT );
            }

            /* Re-enable real-time timer to time clock reinitialisation delay */
            rtt_init( RTT, RTT_CLOCK_PRESCALER );

            /* Reinit fast clock. This takes ~19ms, but the timing has been compensated */
            init_clocks();

            /* Disable unused clock to save power */
            pmc_osc_disable_fastrc();

            /* Restore system I/O pins */
            matrix_set_system_io( system_io_backup_value );

            /* Restore WLAN bus */
            //platform_bus_exit_powersave();

//            /* Restore platform-specific settings */
//            platform_exit_powersave();

            /* Add clock reinitialisation delay to elapsed cycles */
            elapsed_cycles += rtt_read_timer_value( RTT );

            /* Disable RTT to save power */
            RTT->RTT_MR = (uint32_t)( 1 << 20 );
        }
    }

    /* Start atomic operation */
    DISABLE_INTERRUPTS;

    /* Switch SysTick back on */
    SysTick->CTRL |= ( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );

    /* Clear flag indicating interrupt triggered by wake up pin */
    wake_up_interrupt_triggered = false;

    /* End atomic operation */
    ENABLE_INTERRUPTS;

    /* Return total time in milliseconds */
    return CYCLES_TO_MS( elapsed_cycles );
}
#endif /* MICO_DISABLE_MCU_POWERSAVE */


void platform_mcu_enter_standby(uint32_t secondsToWakeup)
{
  platform_log("unimplemented");
  return;
}


/******************************************************
 *         IRQ Handlers Definition & Mapping
 ******************************************************/


/******************************************************
 *        Interrupt Service Routine Definitions
 ******************************************************/

#ifndef MICO_DISABLE_MCU_POWERSAVE
MICO_RTOS_DEFINE_ISR(RTT_Handler)
{
    // __set_PRIMASK( 1 );
    // rtt_get_status( RTT );
    // rtt_disable_interrupt( RTT, RTT_MR_ALMIEN );
}
#endif /* MICO_DISABLE_MCU_POWERSAVE */









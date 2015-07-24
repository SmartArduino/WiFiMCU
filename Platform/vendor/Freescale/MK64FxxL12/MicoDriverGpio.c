/**
******************************************************************************
* @file    MicoDriverGpio.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide GPIO driver functions.
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


#include "MICOPlatform.h"
#include "MICORTOS.h"
#include "k64_gpio.h"

#include "platform.h"
#include "platform_common_config.h"
#include "ksdk_platform.h"
#include "board.h"
/******************************************************
*                    Constants
******************************************************/

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
*               Variables Definitions
******************************************************/
extern platform_pin_mapping_t gpio_mapping[];

/******************************************************
*               Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/

OSStatus MicoGpioInitialize( mico_gpio_t gpio, mico_gpio_config_t configuration )
{
  gpio_input_pin_user_config_t input;
  gpio_output_pin_user_config_t output;

  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;

  if (gpio == WL_RESET) {
    printf("reset %x pin intilized as %d\r\n",gpio_mapping[gpio].pinName, configuration);
  }
  
  MicoMcuPowerSaveConfig(false);
  switch(configuration) {
  case INPUT_PULL_UP:
    input.pinName = gpio_mapping[gpio].pinName;
    input.config.isPullEnable = true;
    input.config.pullSelect = kPortPullUp;
    input.config.isPassiveFilterEnabled = false;
    input.config.interrupt = kPortIntDisabled;
    GPIO_DRV_InputPinInit(&input);
    break;
  case INPUT_PULL_DOWN:
    input.pinName = gpio_mapping[gpio].pinName;
    input.config.isPullEnable = true;
    input.config.pullSelect = kPortPullDown;
    input.config.isPassiveFilterEnabled = false;
    input.config.interrupt = kPortIntDisabled;
    GPIO_DRV_InputPinInit(&input);
    break;
  case INPUT_HIGH_IMPEDANCE:
    input.pinName = gpio_mapping[gpio].pinName;
    input.config.isPullEnable = false;
    input.config.pullSelect = kPortPullDown;
    input.config.isPassiveFilterEnabled = false;
    input.config.interrupt = kPortIntDisabled;
    GPIO_DRV_InputPinInit(&input);
    break;
  case OUTPUT_PUSH_PULL:
    output.pinName = gpio_mapping[gpio].pinName;
    output.config.outputLogic = 1;
    output.config.slewRate = kPortSlowSlewRate;
    output.config.isOpenDrainEnabled = false;
    output.config.driveStrength = kPortLowDriveStrength;
    if (gpio == WL_RESET) {
        printf("reset pin intilized as output\r\n");
      }
    GPIO_DRV_OutputPinInit(&output);
    break;
  case OUTPUT_OPEN_DRAIN_NO_PULL:
    output.pinName = gpio_mapping[gpio].pinName;
    output.config.outputLogic = 1;
    output.config.slewRate = kPortSlowSlewRate;
    output.config.isOpenDrainEnabled = true;
    output.config.driveStrength = kPortLowDriveStrength;
    GPIO_DRV_OutputPinInit(&output);
    break;
  case OUTPUT_OPEN_DRAIN_PULL_UP:
    output.pinName = gpio_mapping[gpio].pinName;
    output.config.outputLogic = 1;
    output.config.slewRate = kPortSlowSlewRate;
    output.config.isOpenDrainEnabled = true;
    output.config.driveStrength = kPortLowDriveStrength;
    GPIO_DRV_OutputPinInit(&output);
    break;
  default:
    MicoMcuPowerSaveConfig(true);
    return kUnsupportedErr;
  }
  
  
  MicoMcuPowerSaveConfig(true);
  return kNoErr;
}

OSStatus MicoGpioFinalize( mico_gpio_t gpio )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;
  return kNoErr;
}

OSStatus MicoGpioOutputHigh( mico_gpio_t gpio )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;
    GPIO_DRV_SetPinOutput(gpio_mapping[gpio].pinName);

    
  return kNoErr;
}

OSStatus MicoGpioOutputLow( mico_gpio_t gpio )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;

  GPIO_DRV_ClearPinOutput(gpio_mapping[gpio].pinName);
  return kNoErr;
}

OSStatus MicoGpioOutputTrigger( mico_gpio_t gpio )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;
  GPIO_DRV_TogglePinOutput(gpio_mapping[gpio].pinName);
  return kNoErr;    
}

bool MicoGpioInputGet( mico_gpio_t gpio )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return 0;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;
  return GPIO_DRV_ReadPinInput(gpio_mapping[gpio].pinName);
}

OSStatus MicoGpioEnableIRQ( mico_gpio_t gpio, mico_gpio_irq_trigger_t trigger, mico_gpio_irq_handler_t handler, void* arg )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;
  return gpio_irq_enable( GPIO_EXTRACT_PORT(gpio_mapping[gpio].pinName), 
    GPIO_EXTRACT_PIN(gpio_mapping[gpio].pinName), 
    trigger, handler, arg );
}

OSStatus MicoGpioDisableIRQ( mico_gpio_t gpio )
{
  if(gpio == (mico_gpio_t)MICO_GPIO_UNUSED ) return kUnsupportedErr;
  if (gpio_mapping[gpio].pinName == GPIO_PINS_OUT_OF_RANGE)
    return kUnsupportedErr;
  return gpio_irq_disable( GPIO_EXTRACT_PORT(gpio_mapping[gpio].pinName), 
    GPIO_EXTRACT_PIN(gpio_mapping[gpio].pinName) );
}

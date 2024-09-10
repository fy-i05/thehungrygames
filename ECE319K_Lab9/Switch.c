/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00050081;// Button1: English
    IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00050081;// Button3: pause
    IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00050081;// Button4 : EAT/SWALLOW
    IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00050081;// Button2 : Spanish

    // write this
  
}
// return current state of switches
uint32_t Switch_In(void){
    uint32_t Input;
    Input=GPIOB->DIN31_0 ;
    Input = Input >>13; //
    Input = Input & 0x59;
  return Input; //replace this your code
}


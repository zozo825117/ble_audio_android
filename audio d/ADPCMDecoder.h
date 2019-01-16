/*****************************************************************************
* File Name: ADPCMDecoder.h
* Version
*
* Description:
* Handles decompression of nibble into 2 byte word using ADPCM decoder.
*
* Owner:
* SIRK
*
* Related Document:
*
*
* Hardware Dependency:
* CYPRESS BLE Dongle
*
* Code Tested With:
* PSoC Creator
*
******************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/
#ifndef _ADPCMDECODER_H_
#define _ADPCMDECODER_H_
#include <stdint.h>
#include <stdbool.h>

typedef  int16_t int16;
typedef  uint16_t uint16;
typedef  int8_t  int8;
typedef  uint8_t uint8;
typedef  int32_t  int32;
typedef  uint32_t uint32;



/*****************************************************************************
* MACRO Definition
*****************************************************************************/
/*Audio sample should be in 32676 to -32768*/
#define AUDIO_SAMPLE_MAX                         (32767)
#define AUDIO_SAMPLE_MIN                         (-32768)

/*Maximum value of index*/
#define ADPCM_INDEX_MAX                          (88)

/*Audio packet's first byte is lookup table index*/
#define ADPCM_PREV_INDEX                         (0u)

/*Audio packet's 2nd and 3rd byte contains 16 bit audio sample.*/
#define ADPCM_PREV_SAMPLE_BYTE_0                 (1u)
#define ADPCM_PREV_SAMPLE_BYTE_1                 (2u)

/* Bit position macros */
#define BIT_0_POSITION                           (0u)
#define BIT_1_POSITION                           (1u)
#define BIT_2_POSITION                           (2u)
#define BIT_3_POSITION                           (3u)
#define BIT_4_POSITION                           (4u)
#define BIT_5_POSITION                           (5u)
#define BIT_6_POSITION                           (6u)
#define BIT_7_POSITION                           (7u)
#define BIT_8_POSITION                           (8u)

/* Bit mask macros */
#define BIT_0_MASK                              ((uint32)1<<BIT_0_POSITION)
#define BIT_1_MASK                              ((uint32)1<<BIT_1_POSITION)
#define BIT_2_MASK                              ((uint32)1<<BIT_2_POSITION)
#define BIT_3_MASK                              ((uint32)1<<BIT_3_POSITION)
#define BIT_4_MASK                              ((uint32)1<<BIT_4_POSITION)
#define BIT_5_MASK                              ((uint32)1<<BIT_5_POSITION)
#define BIT_6_MASK                              ((uint32)1<<BIT_6_POSITION)
#define BIT_7_MASK                              ((uint32)1<<BIT_7_POSITION)
#define BIT_8_MASK                              ((uint32)1<<BIT_8_POSITION)

/*Lower Nibble mask*/
#define LOWER_NIBBLE_MASK                       (0xFu)

/*Lower Byte mask*/
#define LOWER_BYTE_MASK                         (0xFFu)
/*****************************************************************************
* Data Struct Definition
*****************************************************************************/
/*Structure to store the last voice sample details*/
typedef struct ADPCMState {

    /*Predicted sample */
    int16 prevSample;

    /*Index into step size table*/
    uint8 prevIndex;
 } ADPCMState;

/*****************************************************************************
* Global Variable Declaration
*****************************************************************************/
extern ADPCMState state;

/*****************************************************************************
* External Function Prototypes
*****************************************************************************/

/*******************************************************************************
* Function Name: ADPCMDecoder
********************************************************************************
*
* Summary:
*  ADPCM Decoder function.
*
* Parameters:
*  uint8 : 4 bit ADPCM code in the lower nibble.
*
* Return:
*  int16 : 16 bit decoded voice sample.
*
* Theory:
* The ADPCM algorithm takes advantage of the high correlation between
* consecutive speech samples, which enables future sample values to be
* predicted. Instead of encoding the speech sample, ADPCM encodes the
* difference between a predicted sample and the speech sample. This
* method provides more efficient compression with a reduction in the
* number of bits per sample, yet preserves the overall quality of the
* speech signal.
*
* Side Effects:
*  None
*
* Note:
*  none
*
*******************************************************************************/
extern  int16 ADPCMDecoder(const uint8 adpcmCode);
#endif /* _ADPCMDECODER_H_ */
/* [] END OF FILE */

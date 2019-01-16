/*****************************************************************************
* File Name: ADPCMDecoder.c
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
#include "ADPCMDecoder.h"

/*Index table*/
static const int8 indexTable[16] = {
 -1, -1, -1, -1, 2, 4, 6, 8,
 -1, -1, -1, -1, 2, 4, 6, 8
};

/*Step size look up table*/
static const int16 stepSizeTable[ADPCM_INDEX_MAX + 1] = {
 7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
 50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
 130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
 337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/*Structure for previous predicted sample and quantizer step size table index*/
ADPCMState state = {0, 0};

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
int16 ADPCMDecoder(const uint8 adpcmCode )
{    
    /* Index into step size table */
    int8 Index;
    int8 adpcmCodeIndex=0; 

    /*Quantizer step size*/
    int16 step; 

    /*Predicted Sample */
    int32 predSample;

    /*Predicted difference */
    int16 difference; 

    /*Previous predicted sample.*/
    predSample = state.prevSample;

    /*Previous quantizer step size index.*/
    Index = state.prevIndex;

    /*Step size*/
    step = stepSizeTable[Index];

    /*Get the difference using ADPCM code and the quantizer step size*/
    difference = step >> BIT_3_POSITION;

    if(adpcmCode & BIT_1_MASK)
    {
        difference += step;
        adpcmCodeIndex |= BIT_2_MASK;
    }

    if(adpcmCode & BIT_2_MASK)
    {
        difference += (step >> BIT_1_POSITION);
        adpcmCodeIndex |= BIT_1_MASK;
    }

    if(adpcmCode & BIT_3_MASK)
    {
        difference += (step >> BIT_2_POSITION);
        adpcmCodeIndex |= BIT_0_MASK;
    }

    /* Add the difference to the predicted sample*/
    if(adpcmCode & BIT_0_MASK)
    {
        predSample -= difference;
        adpcmCodeIndex |= BIT_3_MASK;
    }
    else
    {
        predSample += difference;
    }

    /*Check overflow of the predicted sample*/
    if(predSample > AUDIO_SAMPLE_MAX)
    {
        predSample = AUDIO_SAMPLE_MAX;
    }
    else if(predSample < AUDIO_SAMPLE_MIN)
    {
        predSample = AUDIO_SAMPLE_MIN;
    }

    /* Get the new quantizer step size by adding the previous Index and a table lookup using the ADPCM code*/
    Index += indexTable[(adpcmCodeIndex & (BIT_0_MASK | BIT_1_MASK | BIT_2_MASK | BIT_3_MASK))];

    /* Check overflow of the quantizer step size index*/
    if(Index < 0)
    {
        Index = 0;
    }
    else if(Index > ADPCM_INDEX_MAX)
    {
        Index = ADPCM_INDEX_MAX;
    }

    /*Save the current predicted sample and index.*/
    state.prevSample = (int16)predSample;
    state.prevIndex = (uint8)Index;

    return((int16)predSample);
}
/* [] END OF FILE */

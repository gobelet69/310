/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <math.h>

#define PI 3.14
#define N 100
float signal[N];

uint16_t i = 0;
uint8_t value;

CY_ISR (myISR){
    value = 128 + 128*signal[i];
    DAC_SetValue(value);
    i++; 
    if (i==N){i=0;}    
    Timer_DAC_ISR_ReadStatusRegister(); //Do not forget to reset the register
}

int main(void)
{
    uint16_t val_adc = 0;
    uint16_t cnt = 0;
    
    for (int j=0; j<N; j++){
        signal[j] = sin(2*PI*j/N);
    }
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    // Start ADC
    DAC_Start();
    DAC_SetValue(0);
    ISR_Timer_DAC_StartEx(myISR);
    Timer_Start();
    Timer_DAC_ISR_Start();
    
    Timer_DAC_Start();
    LCD_Start();
    LCD_Position(1,0);
    
    for(;;)
    {
        
        if(0x80 & Timer_ReadStatusRegister()){ //In case of overflow
            if (cnt < 1000){
                cnt++;
            }
            else{
                LED1_Write(!LED1_Read());
                cnt = 0; //Reset counter
            }
        }
        if(0x80 & Timer_DAC_ReadStatusRegister()){
            value = 128 + 128*signal[i];
            DAC_SetValue(value);
            i++;
            if (i==N){i=0;}    
            Timer_DAC_ReadStatusRegister();
        }
        //Try to add a CyDelay here. What happens ?
    }
}



/* [] END OF FILE */

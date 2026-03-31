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
#include "keypad.h"

void assignment2(void); //Prototypes of functions
void assignment3(uint8_t* sw2_pressed, uint8_t* ctr);
void assignment4(char* key, char* last_key);
void assignment5(uint32_t* potVal, uint32_t* photoResVal);

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    LCD_Start();
    LCD_ClearDisplay();
    keypadInit();
    Mux_Start();
    ADC_Start();
    
    uint8_t sw2_pressed     = 0;
    uint8_t ctr             = 0;
    char key                = 'z';
    char last_key           = 'z';
    uint32_t potVal = 0;
    uint32_t photoResVal = 0;
    
    GPIOJ12_Write(0);
    GPIOJ13_Write(0);
    GPIOJ14_Write(0);
    
    for(;;) 
    {
        /* Place your application code here. */
        //assignment2();
        //assignment3(&sw2_pressed, &ctr);
        //assignment4(&key, &last_key);
        assignment5(&potVal, &photoResVal); //Comment 3 previous lines if you use this one to prevent LCD conflict
        
        CyDelay(50); //Do not abuse this function
        
    }
}

void assignment2(){
// Assignment 2 : all the LEDs switched on when SW1 pressed.
    LED1_Write(SW1_Read()); 
    LED2_Write(SW1_Read()); 
    LED3_Write(SW1_Read()); 
    LED4_Write(SW1_Read()); 
}

void assignment3(uint8_t* sw2_pressed, uint8_t* ctr){
/* Assignment 3 : here we detect a rising edge on SW2 and display the number
    of time SW2 has been pressed using ctr */
    
    if (SW2_Read()){
        *sw2_pressed = 1;
    }
    if ((!SW2_Read()) && *sw2_pressed){
        *sw2_pressed = 0;
        (*ctr)++;
    }
    LCD_Position(0,0); LCD_PrintString("SW2 coun");
    LCD_Position(1,0); LCD_PrintString("t:"); //Use PrintString to print strings
    LCD_PrintNumber(*ctr); //Use PrintNumber for numbers
}

void assignment4(char* key, char* last_key){
 // Assignment 4 : display the pressed keyboard character on the LCD screen
    *key = keypadScan();
    
    if(SW3_Read()){
        *key = 'o';
    }
    
    if (*key != 'z'){ 
        if (*key != *last_key){ //Display only if new character is pressed
            LCD_Position(1,4); LCD_PrintString("- ");
            LCD_PutChar(*key);
            *last_key = *key;
        }
    }
}

void assignment5(uint32_t* potVal, uint32_t* photoResVal){
    /* Assignment 5 : uses the ADC to convert the voltage from the potentiometer
    and the one from the photoresistor (assumed connected to J12 = R10). Light the LEDs according
    to the value of the potentiometer.*/
    
    Mux_Select(0);
    CyDelay(10); //This one let some time for the switch to occur. Otherwise conversion does not work properly
    ADC_StartConvert();
    if(ADC_IsEndConversion(ADC_WAIT_FOR_RESULT)){
        *potVal = ADC_GetResult32();
    }
    Mux_Select(1);
    CyDelay(10);
    ADC_StartConvert();
    if(ADC_IsEndConversion(ADC_WAIT_FOR_RESULT)){
        *photoResVal = ADC_GetResult32();
    }
    
    LCD_ClearDisplay();
    LCD_Position(0,0);
    *potVal = (*potVal*5000/(0xFFFF)); //Get a value between 0 and 5000 mV
    LCD_PrintNumber(*potVal);
    LCD_PrintString(" mV");
    
    if(*potVal > 0 && *potVal < 1000){
        LED1_Write(0); LED2_Write(0); LED3_Write(0); LED4_Write(0); 
    }
    else if(*potVal >= 1000 && *potVal < 2000){
        LED1_Write(1); LED2_Write(0); LED3_Write(0); LED4_Write(0); 
    }
    else if(*potVal >= 2000 && *potVal < 3000){
        LED1_Write(1); LED2_Write(1); LED3_Write(0); LED4_Write(0); 
    }
    else if(*potVal >= 3000 && *potVal < 4000){
        LED1_Write(1); LED2_Write(1); LED3_Write(1); LED4_Write(0); 
    }
    else if(*potVal >= 4000){
        LED1_Write(1); LED2_Write(1); LED3_Write(1); LED4_Write(1); 
    }
    
    LCD_Position(1,0);
    LCD_PrintString("- ");
    LCD_PrintNumber(*photoResVal); 
    
}



/* [] END OF FILE */

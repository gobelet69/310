#include "project.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "keypad.h"

/* ========================================================= */
/* Constantes et Paramètres de Configuration                 */
/* ========================================================= */
/* --- Servomoteurs --- */
#define SERVO_0_DEG      1500u  
#define SERVO_180_DEG    1700u  
#define SERVO_STEP       50u    
#define SERVO_DELAY      10u    
/* --- Capteurs Optiques (Photorésistances) --- */
#define SEUIL_LUMIERE    1000u  
#define HYSTERESIS       100u   
#define ADC_CH_SENSOR_1  0      
#define ADC_CH_SENSOR_2  1      
/* --- États Matériels Explicites --- */
#define BTN_PRESSED      0      
#define BTN_RELEASED     1      
#define LED_ON           1      
#define LED_OFF          0      
/* --- Paramètres du Jeu --- */
#define LCD_TIMEOUT      20u    
#define MAX_SPEED        13.0f  
#define SPEED_INC        0.003f 
#define DIST_MULT        3.0f   
#define SCORE_MULT       0.025f 

/* ========================================================= */
/* Structures de Données                                     */
/* ========================================================= */
typedef struct {
    float  distanceRan;   
    float  currentSpeed;  
    uint16 score;         
    bool   isStarted;     
    uint16 jumpCount;     
    uint16 duckCount;     
} GameState_t;

/* ========================================================= */
/* Variables Globales (Privées au fichier via static)        */
/* ========================================================= */
static GameState_t game;
/* --- Variables de communication UART (Buffer) --- */
#define RX_BUFFER_SIZE 16
static char  rxBuffer[RX_BUFFER_SIZE];
static uint8 rxIndex        = 0;
static bool  rxMessageReady = false;
static bool  uartError      = false;
/* --- Variables pour l'affichage LCD (Max 8 chars pour l'action) --- */
static char  lcdMsg[9]  = "        "; 
static uint8 lcdTimer   = 0;
/* --- Variables d'état (Mémoire des capteurs) --- */
static uint8 lastSW4 = BTN_RELEASED;
static uint8 lastSW3 = BTN_RELEASED;
static uint8 lastSW2 = BTN_RELEASED;
static bool  lastPhoto1Sombre = false;
static bool  lastPhoto2Sombre = false;
/* --- Variable d'état clavier matriciel --- */
static char  lastKey = 'z';

/* ========================================================= */
/* Déclarations des fonctions (Prototypes)                   */
/* ========================================================= */
static void System_Init(void);
static void Game_Reset(void);
static void Game_UpdateLogic(void);
static void Dino_Jump(void);
static void Dino_Duck(void);
static void Inputs_Process(void);
static void Keypad_Process(void);
static void LCD_RequestMsg(const char* msg);
static void LCD_ProcessUpdate(void);
static void UART_Process(void);
CY_ISR_PROTO(isr_uart_Handler);

/* ========================================================= */
/* Logique de Jeu Principale                                 */
/* ========================================================= */
static void Game_Reset(void)
{
    game.isStarted    = false;
    game.distanceRan  = 0.0f;
    game.currentSpeed = 6.0f;
    game.score        = 0;
    game.jumpCount    = 0;
    game.duckCount    = 0;
    
    /* --- Clignotement des 4 LEDs x3 --- */
    for (uint8 i = 0; i < 3; i++)
    {
        LED_1_Write(LED_ON);
        LED_2_Write(LED_ON);
        LED_3_Write(LED_ON);
        LED_4_Write(LED_ON);
        CyDelay(200u);
        LED_1_Write(LED_OFF);
        LED_2_Write(LED_OFF);
        LED_3_Write(LED_OFF);
        LED_4_Write(LED_OFF);
        CyDelay(200u);
    }
    
    LCD_RequestMsg("Reset   ");
    
    UART_PutString("Action: RESET\r\n");
}

static void Game_UpdateLogic(void)
{
    if (game.isStarted) 
    {
        game.distanceRan += game.currentSpeed * DIST_MULT; 
        if (game.currentSpeed < MAX_SPEED) 
        {
            game.currentSpeed += SPEED_INC; 
        }
        game.score = (uint16)(game.distanceRan * SCORE_MULT); 
    }
}

/* ========================================================= */
/* Actions du Dinosaure (Logique + Écran + Matériel + UART)  */
/* ========================================================= */
static void Dino_Jump(void)
{
    char msgBuffer[9];
    if (!game.isStarted) game.isStarted = true;
    game.jumpCount++;
    sprintf(msgBuffer, "Jump %3u", game.jumpCount);
    LCD_RequestMsg(msgBuffer); 
    UART_PutString("Action: JUMP\r\n");
    LED_1_Write(LED_ON);
    LED_2_Write(LED_ON);
    
    for (uint16 pos = SERVO_180_DEG; pos >= SERVO_0_DEG; pos -= SERVO_STEP)
    {
        PWM_1_WriteCompare1(pos);
        CyDelay(SERVO_DELAY);
    }
    for (uint16 pos = SERVO_0_DEG; pos <= SERVO_180_DEG; pos += SERVO_STEP)
    {
        PWM_1_WriteCompare1(pos);
        CyDelay(SERVO_DELAY);
    }
    
    LED_1_Write(LED_OFF);
    LED_2_Write(LED_OFF);
}

static void Dino_Duck(void)
{
    char msgBuffer[9];
    game.duckCount++;
    sprintf(msgBuffer, "Duck %3u", game.duckCount);
    LCD_RequestMsg(msgBuffer); 
    UART_PutString("Action: DUCK\r\n");
    LED_3_Write(LED_ON);
    LED_4_Write(LED_ON);
    
    for (uint16 pos = SERVO_180_DEG; pos >= SERVO_0_DEG; pos -= SERVO_STEP)
    {
        PWM_2_WriteCompare1(pos);
        CyDelay(SERVO_DELAY);
    }
    for (uint16 pos = SERVO_0_DEG; pos <= SERVO_180_DEG; pos += SERVO_STEP)
    {
        PWM_2_WriteCompare1(pos);
        CyDelay(SERVO_DELAY);
    }
    
    LED_3_Write(LED_OFF);
    LED_4_Write(LED_OFF);
}

/* ========================================================= */
/* Gestion des Entrées (Boutons & Capteurs Optiques)         */
/* ========================================================= */
static void Inputs_Process(void)
{
    uint8 currentSW4 = SW4_Read();
    uint8 currentSW3 = SW3_Read();
    uint8 currentSW2 = SW2_Read(); 
    
    bool jumpTriggered  = (lastSW4 == BTN_RELEASED && currentSW4 == BTN_PRESSED);
    bool duckTriggered  = (lastSW3 == BTN_RELEASED && currentSW3 == BTN_PRESSED);
    bool resetTriggered = (lastSW2 == BTN_RELEASED && currentSW2 == BTN_PRESSED);

    ADC_SAR_Seq_IsEndConversion(ADC_SAR_Seq_WAIT_FOR_RESULT);
    int16 val1 = ADC_SAR_Seq_GetResult16(ADC_CH_SENSOR_1);
    int16 val2 = ADC_SAR_Seq_GetResult16(ADC_CH_SENSOR_2);

    bool photo1Sombre = lastPhoto1Sombre;
    bool photo2Sombre = lastPhoto2Sombre;

    if (val1 < (SEUIL_LUMIERE - HYSTERESIS)) photo1Sombre = true;
    else if (val1 > (SEUIL_LUMIERE + HYSTERESIS)) photo1Sombre = false;

    if (val2 < (SEUIL_LUMIERE - HYSTERESIS)) photo2Sombre = true;
    else if (val2 > (SEUIL_LUMIERE + HYSTERESIS)) photo2Sombre = false;

    if (lastPhoto1Sombre == false && photo1Sombre == true) jumpTriggered = true;
    if (lastPhoto2Sombre == false && photo2Sombre == true) duckTriggered = true;

    if (resetTriggered)
    {
        Game_Reset();
    }
    else if (jumpTriggered)
    {
        Dino_Jump();
    }
    else if (duckTriggered)
    {
        Dino_Duck();
    }

    lastSW4 = currentSW4;
    lastSW3 = currentSW3;
    lastSW2 = currentSW2;
    lastPhoto1Sombre = photo1Sombre;
    lastPhoto2Sombre = photo2Sombre;
}

/* ========================================================= */
/* Gestion du Clavier Matriciel                              */
/* ========================================================= */
static void Keypad_Process(void)
{
    char key = keypadScan();

    if (key != 'z' && key != lastKey)
    {
        if (key == '2')
        {
            Dino_Jump();
        }
        else if (key == '8')
        {
            Dino_Duck();
        }
        else if (key == '0')
        {
            Game_Reset();
        }
    }

    lastKey = key;
}

/* ========================================================= */
/* Affichage LCD (Spécifique Displaytech 161A : 8x2 simulé)  */
/* ========================================================= */
static void LCD_RequestMsg(const char* msg)
{
    for (int i = 0; i < 8; i++)
    {
        lcdMsg[i] = msg[i] ? msg[i] : ' ';
    }
    lcdMsg[8] = '\0';
    lcdTimer  = LCD_TIMEOUT; 
}

static void LCD_ProcessUpdate(void)
{
    char leftMsg[9]  = "        ";
    char rightMsg[9] = "        ";

    if (lcdTimer > 0)
    {
        for(int i = 0; i < 8; i++) leftMsg[i] = lcdMsg[i];
        lcdTimer--;
    }
    else if (!game.isStarted)
    {
        sprintf(leftMsg, "Pret?   "); 
    }

    sprintf(rightMsg, "|%7u", game.score);

    LCD_Position(0,0);
    for(int i = 0; i < 8; i++) LCD_PutChar(leftMsg[i]);
    
    LCD_Position(1,0);
    for(int i = 0; i < 8; i++) LCD_PutChar(rightMsg[i]);
}

/* ========================================================= */
/* Gestion de la communication série (UART)                  */
/* ========================================================= */
CY_ISR(isr_uart_Handler)
{
    uint8 status = 0;
    do
    {
        status = UART_ReadRxStatus();
        
        if ((status & UART_RX_STS_PAR_ERROR)  ||
            (status & UART_RX_STS_STOP_ERROR) ||
            (status & UART_RX_STS_BREAK)      ||
            (status & UART_RX_STS_OVERRUN))
        {
            uartError = true;
        }
        
        if ((status & UART_RX_STS_FIFO_NOTEMPTY) != 0)
        {
            char c = UART_ReadRxData();
            UART_PutChar(c); 
            if (c == '\r' || c == '\n')
            {
                if (rxIndex > 0)
                {
                    rxBuffer[rxIndex] = '\0'; 
                    rxMessageReady = true;    
                    rxIndex = 0;              
                }
            }
            else
            {
                if (rxIndex < (RX_BUFFER_SIZE - 1))
                {
                    rxBuffer[rxIndex] = c;
                    rxIndex++;
                }
            }
        }
    } while((status & UART_RX_STS_FIFO_NOTEMPTY) != 0);
}

static void UART_Process(void)
{
    if (uartError)
    {
        uartError = false;
        LCD_RequestMsg("Err UART"); 
    }
    
    if (rxMessageReady)
    {
        rxMessageReady = false;
        if (strcmp(rxBuffer, "jump") == 0 || strcmp(rxBuffer, "JUMP") == 0)
        {
            LCD_RequestMsg("Rx:JUMP ");
            LCD_ProcessUpdate();
            CyDelay(800);
            Dino_Jump();
        }
        else if (strcmp(rxBuffer, "duck") == 0 || strcmp(rxBuffer, "DUCK") == 0)
        {
            LCD_RequestMsg("Rx:DUCK ");
            LCD_ProcessUpdate();
            CyDelay(800);
            Dino_Duck();
        }
        else if (strcmp(rxBuffer, "reset") == 0 || strcmp(rxBuffer, "RESET") == 0)
        {
            LCD_RequestMsg("Rx:RESET");
            LCD_ProcessUpdate();
            CyDelay(800);
            Game_Reset();
        }
        else
        {
            LCD_RequestMsg("Cmd Err "); 
            UART_PutString("Action: ERROR - Commande inconnue\r\n");
        }
        
        rxBuffer[0] = '\0'; 
    }
}

/* ========================================================= */
/* Initialisation Globale du Système                         */
/* ========================================================= */
static void System_Init(void)
{
    CyGlobalIntEnable; 
    
    LCD_Start();
    UART_Start();
    isr_uart_StartEx(isr_uart_Handler);
    
    PWM_1_Start();
    PWM_2_Start();
    ADC_SAR_Seq_Start();
    ADC_SAR_Seq_StartConvert();
    
    PWM_1_WriteCompare1(SERVO_180_DEG);
    PWM_2_WriteCompare1(SERVO_0_DEG);
    LED_1_Write(LED_OFF);
    LED_2_Write(LED_OFF);
    LED_3_Write(LED_OFF);
    LED_4_Write(LED_OFF);
    
    keypadInit();

    Game_Reset(); 
    
    CyDelay(50u);
    
    lastSW4 = SW4_Read();
    lastSW3 = SW3_Read();
    lastSW2 = SW2_Read(); 
    
    ADC_SAR_Seq_IsEndConversion(ADC_SAR_Seq_WAIT_FOR_RESULT);
    lastPhoto1Sombre = (ADC_SAR_Seq_GetResult16(ADC_CH_SENSOR_1) < SEUIL_LUMIERE) ? true : false;
    lastPhoto2Sombre = (ADC_SAR_Seq_GetResult16(ADC_CH_SENSOR_2) < SEUIL_LUMIERE) ? true : false;
}

/* ========================================================= */
/* Fonction Main (Boucle Infinie)                            */
/* ========================================================= */
int main(void)
{
    System_Init();
    
    for(;;)
    {
        Game_UpdateLogic();
        Inputs_Process();
        UART_Process();
        Keypad_Process();
        LCD_ProcessUpdate();
        
        CyDelay(50u); 
    }
}
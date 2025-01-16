#include <xc.h>
#include <stdint.h>
#include <pic18f4520.h>
#include <math.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include <time.h>
// ?????
#pragma config OSC = INTIO67 // ????????RA6?RA7????IO
#pragma config WDT = OFF     // ????????
#pragma config PWRT = OFF    // ??????
#pragma config BOREN = ON    // ??????
#pragma config PBADEN = OFF  // PORTB???????IO
#pragma config LVP = OFF     // ?????????
#pragma config CPD = OFF     // ??????



// ??????
#define _XTAL_FREQ 1000000 // 1MHz

// ???????????CCP1/PWM ???
#define BUZZER_LAT LATCbits.LATC2
#define BUZZER_TRIS TRISCbits.TRISC2
#define RANDOM_RINGTONE_LENGTH 16
unsigned int randomRingtone[RANDOM_RINGTONE_LENGTH];
static const unsigned int NOTES[8] = {524, 588, 660, 698, 784, 880, 988, 1046};

const unsigned int TWINKLE_MELODY[] = {
    524, 524, 784, 784, 880, 880, 784,  // C C G G A A G
    698, 698, 660, 660, 588, 588, 524,  // F F E E D D C
    784, 784, 698, 698, 660, 660, 588,  // G G F F E E D
    784, 784, 698, 698, 660, 660, 588,  // G G F F E E D
    524, 524, 784, 784, 880, 880, 784,  // C C G G A A G
    698, 698, 660, 660, 588, 588, 524   // F F E E D D C
};

#define CANON_MELODY_LENGTH 16
const unsigned int CANON_MELODY[CANON_MELODY_LENGTH] = {
    660, 588, 524, 499, 
    440, 392, 440, 499,  
    660, 588, 524, 499,  
    440, 392, 440, 499,
};
const unsigned int NIGHT_MELODY[48] = {
    392, 640, 640, 640, 588, 640,  
    588, 588, 588, 524, 524, 392, 640, 640, 
    408, 832, 832, 640, 784, 784, 784,  
    698, 698, 640, 588, 588, 588, 640, 640,
    445, 524, 524, 524, 408, 408, 408, 392,
    934, 832, 784, 698,
    640, 698, 408, 445,
    524, 524, 524,
};

#define MODE_IDLE 0
#define MODE_RECORD 1
#define MODE_PLAYBACK 2
int stop = 0;
int first = 1;
int rhythm = 1; //justify melody
int mode = MODE_IDLE;
int idx = 0;
long music[100];
int play_idx = 0;
char str[20];
char mystring[20];
int lenStr = 0;
int musicidx = 0;
// ????
void CCP1_PWM_Init(void);
void PWM_SetFrequency(unsigned long freq);
void interrupt_init(void);
void LED_Set(unsigned char value);
void UART_Write_Text(char* text);
void generateRandomRingtone() {
    srand((unsigned int)(time(NULL) ^ rand()));  
    for (int i = 0; i < RANDOM_RINGTONE_LENGTH; i++) {
        randomRingtone[i] = NOTES[rand() % 8];  
    }
}
void playRingtone(const unsigned int *ringtone, int length) {
    for (int i = 0; i < length; i++) {
        unsigned int freq = ringtone[i];
        PWM_SetFrequency(freq); 
        if(stop == 1){
            break;
        }
        LED_Set(1 << (i % 8));  
        __delay_ms(250);         
        T2CONbits.TMR2ON = 0;   
        __delay_ms(100);         
        T2CONbits.TMR2ON = 1;
    }
    T2CONbits.TMR2ON = 0;        
    LED_Set(0);                 
    __delay_ms(100);             
}
void playCanonRingtone() {
    int length = CANON_MELODY_LENGTH; 
    for (int i = 0; i < length; i++) {
        unsigned int highFreq = CANON_MELODY[i];
    //    unsigned int bassFreq = CANON_BASS[i]; //low part melody
        /* need two buzzer 
        PWM_SetFrequency(bassFreq); 
        LED_Set(1 << ((i + 4) % 8)); 
        __delay_ms(800);             
       */
        PWM_SetFrequency(highFreq);  
        if(stop == 1){
            break;
        }
        LED_Set(1 << (i % 8));       
        __delay_ms(800);             
      if(rhythm == 1){
            __delay_ms(400); 
            T2CONbits.TMR2ON = 0;
            __delay_ms(400);
        }else{
            __delay_ms(200); 
            T2CONbits.TMR2ON = 0;
            __delay_ms(200);    
        }             
        T2CONbits.TMR2ON = 1;
    }
}
void playNightRingtone() {
    int length = 48; 
    for (int i = 0; i < length; i++) {
        unsigned int highFreq = NIGHT_MELODY[i];
    //    unsigned int bassFreq = CANON_BASS[i]; //low part melody
        /* need two buzzer 
        PWM_SetFrequency(bassFreq); 
        LED_Set(1 << ((i + 4) % 8)); 
        __delay_ms(800);             
       */
        PWM_SetFrequency(highFreq);  
        if(stop == 1){
            break;
        }
        LED_Set(1 << (i % 8));               
      if(rhythm == 1){
             if(i == 39 || i == 40 || i == 41 || i == 42 || i == 43 || i == 44){
                __delay_ms(100); 
                T2CONbits.TMR2ON = 0;
                __delay_ms(100);
                 T2CONbits.TMR2ON = 1;
                 continue;
            }
            __delay_ms(200); 
            T2CONbits.TMR2ON = 0;
            __delay_ms(200);
            
           
        }else{
          if(i == 39 || i == 40 || i == 41 || i == 42 || i == 43 || i == 44){
                __delay_ms(50); 
                T2CONbits.TMR2ON = 0;
                __delay_ms(50);
                 T2CONbits.TMR2ON = 1;
                 continue;
            }
            __delay_ms(100); 
            T2CONbits.TMR2ON = 0;
            __delay_ms(100);    
        }             
        T2CONbits.TMR2ON = 1;
    }
     if(rhythm == 1){
         T2CONbits.TMR2ON = 0;
          __delay_ms(200);
          T2CONbits.TMR2ON = 1;
     }else{
         T2CONbits.TMR2ON = 0;
          __delay_ms(100);
          T2CONbits.TMR2ON = 1;
     }      
                
        
    
}
void UART_Initialize() {
  
    TRISCbits.TRISC6 = 1;     //must set to 1          
    TRISCbits.TRISC7 = 1;     //must set to 1   
    RCSTAbits.SPEN = 1;      //must set to 1 
    
    //  Setting baud rate
    TXSTAbits.SYNC = 0;           
    BAUDCONbits.BRG16 = 0;          
    // TXSTAbits.BRGH = 0; 
    TXSTAbits.BRGH = 0; //only set when 250kHz
    SPBRG = 12;    
      //baud rate
    //51 - > 4MHz (0,0,0)1200 BAUD RATE
    // 12 -> 1MHz(0,0,0)1200 baud rate,250kHz (0,0,1)1200 BAUD RATE,125kHz(0,1,1) 2400 baud rate
   //   Serial enable     
    TXSTAbits.TXEN = 1;      //enable transmission
    PIR1bits.TXIF = 1;   //set 1 when TXREG register is empty
    PIR1bits.RCIF = 0;   //when reception complete will be 1           
    PIE1bits.TXIE = 0;  //1 means enable interrupt     
    IPR1bits.TXIP = 0;  //setting interrupt priority     
    PIE1bits.RCIE = 1;      //1 means enable interrupt          
    IPR1bits.RCIP = 0;    //setting interrupt priority
    RCSTAbits.CREN = 1;  //while error detect will be 0

   
    INTCONbits.GIE = 1;    
    INTCONbits.PEIE = 1; 
    UART_Write_Text("Welcome to our musicbox!!\r\n");
    UART_Write_Text("record         record the music you play\r\n");
    UART_Write_Text("play           play the music you record\r\n");
    UART_Write_Text("piano          play piano use adc\r\n");
    UART_Write_Text("ring           play random music cellphone ringtone\r\n");
    UART_Write_Text("star           play twinkle twinkle little star\r\n");
    UART_Write_Text("night          play night song\r\n");
    UART_Write_Text("canon          play canon\r\n");
  //  UART_Write_Text("UART Initialized\r\n");
}

void UART_Write(unsigned char data)  // Output on Terminal
{
    while(!TXSTAbits.TRMT);
    TXREG = data;              //write to TXREG will send data 
}

void UART_Write_Text(char* text) { // Output on Terminal, limit:10 chars
    for(int i=0;text[i]!='\0';i++)
        UART_Write(text[i]);
}

void ClearBuffer(){
    for(int i = 0; i < 20 ; i++)
        mystring[i] = '\0';
    lenStr = 0;
} 
void LED_Set(unsigned char value) {
    LATDbits.LATD4 = (value >> 7) & 0x01;  // ?? RD4 (? 7)
    LATDbits.LATD5 = (value >> 6) & 0x01;  // ?? RD5 (? 6)
    LATDbits.LATD6 = (value >> 5) & 0x01;  // ?? RD6 (? 5)
    LATDbits.LATD7 = (value >> 4) & 0x01;  // ?? RD7 (? 4)
    LATDbits.LATD2 = (value >> 3) & 0x01;  // ?? RD2 (? 3)
    LATDbits.LATD3 = (value >> 2) & 0x01;  // ?? RD3 (? 2)
    LATCbits.LATC4 = (value >> 1) & 0x01;  // ?? RC4 (? 1)
    LATCbits.LATC5 = value & 0x01;         // ?? RC5 (? 0)
}
void LED_Flow(void)
{
 for (int i = 0; i < 8; i++) {
  LED_Set(1 << i);
  __delay_ms(50);
 }
 LED_Set(0);
}
void MyusartRead() {
    unsigned char received_char = RCREG;  
    
    if (received_char == '\r') {
        // go to second line
        UART_Write('\r'); 
        UART_Write('\n'); 
        mystring[lenStr] = '\0'; 
      //  T2CONbits.TMR2ON = 0; //shut up
        LED_Flow();
        musicidx = 0;
        first = 1;
        stop = 1;
        ClearBuffer();  
    } 
    else {
        if (lenStr < sizeof(mystring) - 1) { //avoid longer than buffer(20)
            mystring[lenStr++] = received_char;  
            UART_Write(received_char);  // show what u type
    //        range = atoi(mystring);
        }
    }  
}
void __interrupt(low_priority)  Lo_ISR(void)
{
    if(RCIF)
    {
        if(RCSTAbits.OERR)
        {
            CREN = 0;
            Nop();
            CREN = 1;
        }
        
        MyusartRead();
    }
    
    RCIF = 0; 
   // process other interrupt sources here, if required
    return;
}



char *GetString(){
    return mystring;
}
void LED_Init(void) {
    // ?? LED ???????
    TRISDbits.TRISD4 = 0;  // RD4 ????
    TRISDbits.TRISD5 = 0;  // RD5 ????
    TRISDbits.TRISD6 = 0;  // RD6 ????
    TRISDbits.TRISD7 = 0;  // RD7 ????
    TRISDbits.TRISD2 = 0;  // RD2 ????
    TRISDbits.TRISD3 = 0;  // RD3 ????
    TRISCbits.TRISC4 = 0;  // RC4 ????
    TRISCbits.TRISC5 = 0;  // RC5 ????
}


void interrupt_init(void){
    INTCONbits.GIE = 1; //General INT enable
    INTCONbits.INT0IF = 0; //Clear INT0 flag
    INTCONbits.INT0IE = 1; //Enable INT0 INT
    INTCON2bits.INTEDG0 = 0; //Falling edge trigger for INT0
    RCONbits.IPEN = 1;
}
// ??? ADC ??
void ADC_Init() {
    ADCON0 = 0x01; // ?? ADC??? AN0
    ADCON1 = 0x0E; // ?? AN0 ?????????????
    ADCON2 = 0xA9; // ????ADC ?? Fosc/8
}

unsigned char ADC_ToNoteIndex(unsigned int adc_val)
{
 // ? 0~1023 ??? 8 ??
 return (adc_val * 8) / 1024;
}


// ?? ADC ???????
unsigned int Read_ADC() {
    unsigned int adc_value;
    ADCON0bits.GO = 1; // ????
    while (ADCON0bits.GO_nDONE); // ??????
    adc_value = (ADRESH << 8) | ADRESL; // ?????
    return adc_value;
}

void main(void) {
    LED_Init();
    interrupt_init();
     UART_Initialize();
     int record_idx = 0;
    // ??? I/O ??
     OSCCONbits.IRCF = 0b100;
    BUZZER_TRIS = 0;    // ?? RC2 ????PWM?
    BUZZER_LAT = 0;      // ???????
    TRISBbits.TRISB0 = 1;   // RB0ä½

    INTCON2bits.RBPU = 0;   // ä½¿è½PORTBä¸æé»é»
    // ??? CCP1 ??? PWM
    CCP1_PWM_Init();
    ADC_Init();
    TRISAbits.TRISA0 = 1;
    
   
    // ?? PWM ????? 1000Hz
    PWM_SetFrequency(1000);

    
//if (PORTBbits.RB0 == 0)
    while(1) {
       
            strcpy(str,GetString());
            if(strcmp(str, "piano") == 0){
                if(first == 1){
                    UART_Write_Text("\r\nEnter piano mode");
                    first = 0;
                }
                unsigned int adc_val = Read_ADC(); // ?? ADC ??0-1023?
                unsigned long frequency = 200 + ((unsigned long)adc_val * 1800) / 1023;
                music[idx++]=frequency;
                PWM_SetFrequency(frequency);
                 LED_Set(0xff);
                __delay_ms(500);
           //     T2CONbits.TMR2ON = 0;
                LED_Set(0);
                 __delay_ms(500);
                 T2CONbits.TMR2ON = 1; 
            }else if(strcmp(str, "record") == 0){
                if(first == 1){
                    UART_Write_Text("\r\nEnter record mode");
                    first = 0;
                }
                unsigned int adc_val = Read_ADC();
                unsigned char noteIdx = ADC_ToNoteIndex(adc_val);
                music[record_idx++] = NOTES[noteIdx];
                if (record_idx >= 100)
                 record_idx = 0;
                PWM_SetFrequency(NOTES[noteIdx]);
                LED_Set((1 << (noteIdx + 1)) - 1);
                __delay_ms(1000);
                T2CONbits.TMR2ON = 0;
                __delay_ms(1000);
                T2CONbits.TMR2ON = 1;
            }else if(strcmp(str, "play") == 0){
                if(first == 1){
                    UART_Write_Text("\r\nEnter play mode");
                    first = 0;
                }
                unsigned long freq = music[play_idx];
                if (freq == 0)
                 freq = NOTES[0];
                PWM_SetFrequency(freq);
                for (unsigned char i = 0; i < 8; i++) {
                 if (freq == NOTES[i]) {
                  LED_Set((1 << (i + 1)) - 1);
                  break;
                 }
                }
                __delay_ms(350);
                T2CONbits.TMR2ON = 0;
                __delay_ms(350);
                T2CONbits.TMR2ON = 1;
                play_idx++;
                if (play_idx >= record_idx)
                 play_idx = 0;
            }else if(strcmp(str, "night") == 0){
                 if(first == 1){
                    UART_Write_Text("\r\nEnter night mode");
                    first = 0;
                    __delay_ms(200);
                    T2CONbits.TMR2ON = 1;
                }
                  playNightRingtone();
            }else if(strcmp(str, "star") == 0){
                if(first == 1){
                    UART_Write_Text("\r\nEnter star mode");
                    first = 0;
                    T2CONbits.TMR2ON = 1;
                }
               unsigned long freq = TWINKLE_MELODY[musicidx++];
                PWM_SetFrequency(freq);
                for (unsigned char i = 0; i < 8; i++) {
                 if (freq == NOTES[i]) {
                  LED_Set((1 << (i + 1)) - 1);
                  break;
                 }
                }
                if(rhythm == 1){
                    __delay_ms(250);
                    T2CONbits.TMR2ON = 0;
                    __delay_ms(250);
                     if(musicidx % 7 == 0){
                        __delay_ms(300);
                    }
                    T2CONbits.TMR2ON = 1;
                    if (musicidx >= 42){
                        musicidx = 0;
                        }
                }else{
                     __delay_ms(125);
                    T2CONbits.TMR2ON = 0;
                    __delay_ms(125);
                    if(musicidx % 7 == 0){
                        __delay_ms(300);
                    }
                    T2CONbits.TMR2ON = 1;
                    if (musicidx >= 42){
                        musicidx = 0;
                }
              }                
            }else if(strcmp(str, "ring") == 0){
                if(first == 1){
                    UART_Write_Text("\r\nEnter ring mode");
                    first = 0;
                }
                generateRandomRingtone();  // ??????
                playRingtone(randomRingtone, RANDOM_RINGTONE_LENGTH);
            }else if(strcmp(str, "canon") == 0){
                if(first == 1){
                    UART_Write_Text("\r\nEnter canon mode");
                    first = 0;
                    __delay_ms(400);
                    T2CONbits.TMR2ON = 1;    
                }
                  playCanonRingtone();
            }else{ 
                T2CONbits.TMR2ON = 0;
                stop = 0;
           }
        



       
       
    }

    return;
}

// CCP1 PWM ?????
void CCP1_PWM_Init(void) {
    // ?? CCP1 ? PWM ??
    CCP1CONbits.CCP1M3 = 1;
    CCP1CONbits.CCP1M2 = 1;
    CCP1CONbits.CCP1M1 = 0;
    CCP1CONbits.CCP1M0 = 0;

    // ?? Timer2
    T2CONbits.T2CKPS1 = 1; // ???? 16
    T2CONbits.T2CKPS0 = 1;
    T2CONbits.TMR2ON = 1;  // ?? Timer2

    // ?? PR2?????????????????
    PR2 = 0xFF; // 255

    // ?? PWM ???? 50%????
    CCPR1L = 0x7F; // ?8?
    CCP1CONbits.DC1B1 = 0; // ?2?
    CCP1CONbits.DC1B0 = 0;
}

// ?? PWM ?????
void PWM_SetFrequency(unsigned long freq) {
    // PR2 ????:
    // PWM_freq = Fosc / (4 * (PR2 + 1) * Prescaler)
    // PR2 = (Fosc / (4 * PWM_freq * Prescaler)) - 1
    unsigned long pr2_val = (_XTAL_FREQ) / (4 * freq * 16) - 1;

    if(pr2_val > 255) pr2_val = 255; // PR2 ????255
    PR2 = pr2_val;

    // ?? PWM ???? 50%
    CCPR1L = (pr2_val + 1) / 2; // ?8?
    CCP1CONbits.DC1B1 = 0;      // ?2?
    CCP1CONbits.DC1B0 = 0;
}

void __interrupt(high_priority) H_ISR(){ //button pressed
         if(INTCONbits.INT0IF){
             __delay_ms(25);
             rhythm ^= 1;
              __delay_ms(20);
          }
        INTCONbits.INT0IF = 0;
    }
 
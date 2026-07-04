#include <stm32f10x.h>
#include <string.h>
#include <stdint.h>

/* Delay function prototypes */
void delay_us(unsigned int);
void delay_ms(unsigned int);
void delay_s(unsigned int);

/*---------------- KEYPAD CONNECTIONS ----------------*/
#define row0 0U
#define row1 1U
#define row2 2U
#define row3 3U

#define col0 4U
#define col1 5U
#define col2 6U
#define col3 7U

uint8_t keypad_lut[4][4] =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void keypad_Init(void);
uint32_t colscan(void);
void initrows(void);
void initcols(void);
uint8_t keyscan(void);

/*---------------- LCD CONNECTIONS ----------------*/
#define LCD_RS_PIN    0U
#define LCD_EN_PIN    1U

void LCD_Command(unsigned char);
void LCD_Char(unsigned char);
void LCD_String(unsigned char *);
void LCD_Init(void);
void LCD_SetCursor(unsigned char, unsigned char);
void LCD_UInt(unsigned int);
void LCD_Clear(void);
void LCD_Backspace(void);

/*---------------- OTHER CONNECTIONS ----------------*/
#define BUZZER_ON()   (GPIOB->BSRR = (1U << 2))
#define BUZZER_OFF()  (GPIOB->BRR  = (1U << 2))

#define LED1_ON()     (GPIOA->BSRR = (1U << 8))
#define LED1_OFF()    (GPIOA->BRR  = (1U << 8))

#define LED2_ON()     (GPIOA->BSRR = (1U << 9))
#define LED2_OFF()    (GPIOA->BRR  = (1U << 9))

#define LED3_ON()     (GPIOA->BSRR = (1U << 10))
#define LED3_OFF()    (GPIOA->BRR  = (1U << 10))

#define LED4_ON()     (GPIOA->BSRR = (1U << 11))
#define LED4_OFF()    (GPIOA->BRR  = (1U << 11))

#define START_SWITCH  ((GPIOB->IDR & (1U << 7)) == 0U)

/*---------------- GLOBAL VARIABLES ----------------*/
const char stored_password[] = "1234";
char entered_password[5];
char generated_otp[5];
char entered_otp[5];

unsigned int seed = 1234U;
unsigned int failed_attempts = 0U;

/*---------------- FUNCTION PROTOTYPES ----------------*/
void GPIO_Init_All(void);
void get_password(char *buf, unsigned char len, unsigned char hide);
void get_otp(char *buf, unsigned char len);
void generate_otp(char *otp);
void number_to_string4(unsigned int num, char *buf);
void number_to_string3(unsigned int num, char *buf);
void buzzer_beep_5sec(void);
void buzzer_beep_long(void);
void buzzer_keypress(void);
void leds_locked_state(void);
void unlock_sequence(void);
void lock_sequence(void);
void system_idle(void);
void lockout_mode(void);

/*---------------- MAIN ----------------*/
int main(void)
{
    GPIO_Init_All();
    keypad_Init();
    LCD_Init();

    leds_locked_state();
    system_idle();

    while(1)
    {
        while(!START_SWITCH)
        {
        }

        delay_ms(200U);

        LCD_Clear();
        LCD_SetCursor(0U,0U);
        LCD_String((unsigned char *)"ENTER PASSWORD");
        LCD_SetCursor(1U,0U);
        get_password(entered_password, 4U, 1U);

        if(strcmp(entered_password, stored_password) != 0)
        {
            LCD_Clear();
            LCD_SetCursor(0U,0U);
            LCD_String((unsigned char *)"WRONG PASSWORD");
            buzzer_beep_5sec();
            failed_attempts++;
            delay_ms(500U);

            if(failed_attempts >= 3U)
            {
                lockout_mode();
            }
            else
            {
                system_idle();
            }

            while(START_SWITCH)
            {
            }
            continue;
        }

        failed_attempts = 0U;

        generate_otp(generated_otp);

        LCD_Clear();
        LCD_SetCursor(0U,0U);
        LCD_String((unsigned char *)"OTP:");
        LCD_String((unsigned char *)generated_otp);
        delay_ms(2000U);

        LCD_Clear();
        LCD_SetCursor(0U,0U);
        LCD_String((unsigned char *)"ENTER OTP");
        LCD_SetCursor(1U,0U);
        get_otp(entered_otp, 4U);

        if(strcmp(entered_otp, generated_otp) != 0)
        {
            LCD_Clear();
            LCD_SetCursor(0U,0U);
            LCD_String((unsigned char *)"WRONG OTP");
            buzzer_beep_5sec();
            delay_ms(1000U);
            system_idle();

            while(START_SWITCH)
            {
            }
            continue;
        }

        LCD_Clear();
        LCD_SetCursor(0U,0U);
        LCD_String((unsigned char *)"ACCESS GRANTED");
        delay_ms(1500U);

        unlock_sequence();

        LCD_Clear();
        LCD_SetCursor(0U,0U);
        LCD_String((unsigned char *)"LOCK IS OPEN");
        delay_ms(5000U);

        lock_sequence();

        system_idle();

        while(START_SWITCH)
        {
        }
    }
}

/*---------------- GENERAL GPIO INIT ----------------*/
void GPIO_Init_All(void)
{
    RCC->APB2ENR |= (1U << 2);
    RCC->APB2ENR |= (1U << 3);
    RCC->APB2ENR |= (1U << 4);
    RCC->APB2ENR |= (1U << 0);

    /* PA8-PA11 as output for LEDs */
    GPIOA->CRH &= ~(0xFFFFU);
    GPIOA->CRH |=  (0x2222U);

    /* PB2 as output for buzzer */
    GPIOB->CRL &= ~(0xFU << 8);
    GPIOB->CRL |=  (0x2U << 8);

    /* PB7 as input pull-up for start switch */
    GPIOB->CRL &= ~(0xFU << 28);
    GPIOB->CRL |=  (0x8U << 28);
    GPIOB->ODR |= (1U << 7);
}

/*---------------- PASSWORD / OTP INPUT ----------------*/
void get_password(char *buf, unsigned char len, unsigned char hide)
{
    unsigned char i = 0U;
    unsigned char key;

    while(1)
    {
        key = keyscan();

        if(key == '*')   /* backspace */
        {
            if(i > 0U)
            {
                i--;
                buf[i] = '\0';
                LCD_Backspace();
            }
        }
        else if((key >= '0') && (key <= '9'))
        {
            if(i < len)
            {
                buf[i] = (char)key;

                if(hide)
                {
                    LCD_Char(key);      /* show digit first */
                    delay_ms(100U);    /* 100 Milli second visible */
                    LCD_Backspace();
                    LCD_Char('*');      /* then show * */
                }
                else
                {
                    LCD_Char(key);
                }

                i++;
            }
        }
        else if(key == '#')   /* ENTER key */
        {
            if(i == len)
            {
                buf[len] = '\0';
                break;
            }
            else
            {
                LCD_SetCursor(0U,0U);
                LCD_String((unsigned char *)"ENTER 4 DIGITS  ");
                LCD_SetCursor(1U,i);
            }
        }
    }
}

void get_otp(char *buf, unsigned char len)
{
    unsigned char i = 0U;
    unsigned char key;

    while(1)
    {
        key = keyscan();

        if(key == '*')   /* backspace */
        {
            if(i > 0U)
            {
                i--;
                buf[i] = '\0';
                LCD_Backspace();
            }
        }
        else if((key >= '0') && (key <= '9'))
        {
            if(i < len)
            {
                buf[i] = (char)key;
                LCD_Char(key);
                i++;
            }
        }
        else if(key == '#')   /* ENTER key */
        {
            if(i == len)
            {
                buf[len] = '\0';
                break;
            }
            else
            {
                LCD_SetCursor(0U,0U);
                LCD_String((unsigned char *)"ENTER 4 DIGITS  ");
                LCD_SetCursor(1U,i);
            }
        }
    }
}

/*---------------- OTP GENERATION ----------------*/
void number_to_string4(unsigned int num, char *buf)
{
    buf[0] = (char)((num / 1000U) + '0');
    buf[1] = (char)(((num / 100U) % 10U) + '0');
    buf[2] = (char)(((num / 10U) % 10U) + '0');
    buf[3] = (char)((num % 10U) + '0');
    buf[4] = '\0';
}

void number_to_string3(unsigned int num, char *buf)
{
    buf[0] = (char)((num / 100U) + '0');
    buf[1] = (char)(((num / 10U) % 10U) + '0');
    buf[2] = (char)((num % 10U) + '0');
    buf[3] = '\0';
}

void generate_otp(char *otp)
{
    unsigned int num;

    seed = seed + 137U;
    seed ^= (seed << 7);
    seed ^= (seed >> 9);
    seed ^= (seed << 8);
    seed = seed * 1103515245U + 12345U;

    num = (seed % 9000U) + 1000U;
    number_to_string4(num, otp);
}

/*---------------- BUZZER ----------------*/
void buzzer_beep_5sec(void)
{
    BUZZER_ON();
    delay_ms(5000U);
    BUZZER_OFF();
}

void buzzer_beep_long(void)
{
    BUZZER_ON();
    delay_ms(5000U);
    BUZZER_OFF();
}

void buzzer_keypress(void)
{
    BUZZER_ON();
    delay_ms(40U);
    BUZZER_OFF();
}

/*---------------- LED CONTROL ----------------*/
void leds_locked_state(void)
{
    LED1_ON();
    LED2_ON();
    LED3_ON();
    LED4_ON();
}

void unlock_sequence(void)
{
    LCD_Clear();
    LCD_SetCursor(0U,0U);
    LCD_String((unsigned char *)"LOCK OPENING...");
    delay_ms(500U);

    LED1_OFF(); delay_ms(500U);
    LED2_OFF(); delay_ms(500U);
    LED3_OFF(); delay_ms(500U);
    LED4_OFF(); delay_ms(500U);
}

void lock_sequence(void)
{
    LCD_Clear();
    LCD_SetCursor(0U,0U);
    LCD_String((unsigned char *)"LOCK CLOSING...");
    delay_ms(500U);

    LED1_ON(); delay_ms(500U);
    LED2_ON(); delay_ms(500U);
    LED3_ON(); delay_ms(500U);
    LED4_ON(); delay_ms(500U);
}

void system_idle(void)
{
    LCD_Clear();
    LCD_SetCursor(0U,0U);
    LCD_String((unsigned char *)"PRESS SWITCH TO");
    LCD_SetCursor(1U,0U);
    LCD_String((unsigned char *)"START");
}

void lockout_mode(void)
{
    unsigned int count;
    char buf[4];

    LCD_Clear();
    LCD_SetCursor(0U,0U);
    LCD_String((unsigned char *)"SYSTEM LOCKED");
    buzzer_beep_long();

    for(count = 100U; count > 0U; count--)
    {
        LCD_Clear();
        LCD_SetCursor(0U,0U);
        LCD_String((unsigned char *)"LOCK TIME: ");
        number_to_string3(count, buf);
        LCD_String((unsigned char *)buf);
        delay_s(1U);
    }

    failed_attempts = 0U;
    system_idle();
}

/*---------------- KEYPAD FUNCTIONS ----------------*/
void keypad_Init(void)
{
    RCC->APB2ENR |= (1U << 2);
}

uint32_t colscan(void)
{
    uint32_t c0, c1, c2, c3, retval;

    c0 = (GPIOA->IDR >> col0) & 1U;
    c1 = (GPIOA->IDR >> col1) & 1U;
    c2 = (GPIOA->IDR >> col2) & 1U;
    c3 = (GPIOA->IDR >> col3) & 1U;

    retval = (c0 & c1 & c2 & c3);
    return retval;
}

void initrows(void)
{
    GPIOA->CRL &= ~(0xFFFFU);
    GPIOA->CRL |= ((0x2U << 0) | (0x2U << 4) | (0x2U << 8) | (0x2U << 12));

    GPIOA->BRR = ((1U << row0) | (1U << row1) | (1U << row2) | (1U << row3));
}

void initcols(void)
{
    GPIOA->CRL &= ~(0xFFFF0000U);
    GPIOA->CRL |= ((0x8U << 16) | (0x8U << 20) | (0x8U << 24) | (0x8U << 28));

    GPIOA->ODR |= ((1U << col0) | (1U << col1) | (1U << col2) | (1U << col3));
}

uint8_t keyscan(void)
{
    unsigned char row = 0U, col = 0U;

    initrows();
    initcols();

    while(colscan() == 1U)
    {
    }

    GPIOA->BRR  = (1U << row0);
    GPIOA->BSRR = ((1U << row1) | (1U << row2) | (1U << row3));
    if(colscan() == 0U)
    {
        row = 0U;
        goto colcheck;
    }

    GPIOA->BRR  = (1U << row1);
    GPIOA->BSRR = ((1U << row0) | (1U << row2) | (1U << row3));
    if(colscan() == 0U)
    {
        row = 1U;
        goto colcheck;
    }

    GPIOA->BRR  = (1U << row2);
    GPIOA->BSRR = ((1U << row0) | (1U << row1) | (1U << row3));
    if(colscan() == 0U)
    {
        row = 2U;
        goto colcheck;
    }

    GPIOA->BRR  = (1U << row3);
    GPIOA->BSRR = ((1U << row0) | (1U << row1) | (1U << row2));
    if(colscan() == 0U)
    {
        row = 3U;
        goto colcheck;
    }

colcheck:
    if(((GPIOA->IDR >> col0) & 1U) == 0U)
        col = 0U;
    else if(((GPIOA->IDR >> col1) & 1U) == 0U)
        col = 1U;
    else if(((GPIOA->IDR >> col2) & 1U) == 0U)
        col = 2U;
    else
        col = 3U;

    while(colscan() == 0U)
    {
    }

    delay_ms(20U);
    buzzer_keypress();

    return keypad_lut[row][col];
}

/*---------------- LCD FUNCTIONS ----------------*/
void LCD_Command(unsigned char cmd)
{
    GPIOB->BRR = (1U << LCD_RS_PIN);

    GPIOB->ODR &= 0x00FFU;
    GPIOB->ODR |= ((uint16_t)cmd << 8);

    GPIOB->BSRR = (1U << LCD_EN_PIN);
    delay_us(2U);
    GPIOB->BRR  = (1U << LCD_EN_PIN);
    delay_ms(2U);
}

void LCD_Char(unsigned char data)
{
    GPIOB->BSRR = (1U << LCD_RS_PIN);

    GPIOB->ODR &= 0x00FFU;
    GPIOB->ODR |= ((uint16_t)data << 8);

    GPIOB->BSRR = (1U << LCD_EN_PIN);
    delay_us(2U);
    GPIOB->BRR  = (1U << LCD_EN_PIN);
    delay_ms(2U);
}

void LCD_String(unsigned char *str)
{
    while(*str != '\0')
    {
        LCD_Char(*str++);
    }
}

void LCD_Clear(void)
{
    LCD_Command(0x01U);
    delay_ms(2U);
}

void LCD_Backspace(void)
{
    LCD_Command(0x10U);
    LCD_Char(' ');
    LCD_Command(0x10U);
}

void LCD_Init(void)
{
    RCC->APB2ENR |= (1U << 3);

    /* PB0, PB1 output */
    GPIOB->CRL &= ~((0xFU << 0) | (0xFU << 4));
    GPIOB->CRL |=  ((0x2U << 0) | (0x2U << 4));

    /* PB2 output for buzzer */
    GPIOB->CRL &= ~(0xFU << 8);
    GPIOB->CRL |=  (0x2U << 8);

    /* PB7 input pull-up for switch */
    GPIOB->CRL &= ~(0xFU << 28);
    GPIOB->CRL |=  (0x8U << 28);
    GPIOB->ODR |= (1U << 7);

    /* PB8-PB15 output */
    GPIOB->CRH = 0x22222222U;

    delay_ms(16U);
    LCD_Command(0x30U);
    delay_ms(6U);
    LCD_Command(0x30U);
    delay_ms(1U);
    LCD_Command(0x30U);
    delay_ms(1U);
    LCD_Command(0x38U);
    LCD_Command(0x10U);
    LCD_Command(0x01U);
    LCD_Command(0x06U);
    LCD_Command(0x0CU);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;

    if(row == 0U)
        address = (unsigned char)(0x80U + col);
    else
        address = (unsigned char)(0xC0U + col);

    LCD_Command(address);
}

void LCD_UInt(unsigned int num)
{
    int8_t i = 0;
    unsigned char buffer[10];

    if(num == 0U)
    {
        LCD_Char('0');
    }
    else
    {
        while(num > 0U)
        {
            buffer[i++] = (unsigned char)((num % 10U) + '0');
            num /= 10U;
        }

        for(--i; i >= 0; i--)
        {
            LCD_Char(buffer[i]);
        }
    }
}

/*---------------- DELAY FUNCTIONS ----------------*/
void delay_us(unsigned int dlyUS)
{
    dlyUS *= 12U;
    while(dlyUS--)
    {
        __NOP();
    }
}

void delay_ms(unsigned int dlyMS)
{
    while(dlyMS--)
    {
        delay_us(1000U);
    }
}

void delay_s(unsigned int dlyS)
{
    while(dlyS--)
    {
        delay_ms(1000U);
    }
}

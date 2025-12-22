#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

#include "Lcd.h"

// ===== Bayraklar / Sayaçlar =====
volatile bool ekran_guncelle = false;
volatile bool adc_oku_flag   = false;
volatile bool uart_gonder    = false;
volatile uint8_t sayac1      = 0;

// ===== Saat =====
volatile uint8_t saat = 0, dakika = 0, saniye = 0;

// ===== ADC / Buffer =====
uint32_t adc_degeri;
char saat_buffer[10];
char pot_buffer[6];

// ===== PC -> Tiva RX parser Deðiþkenleri =====
// <ABC> mantýðý için:
static char lcd_temp_buf[4]; // 3 karakter + bosluk
static uint8_t lcd_idx = 0;
static bool veri_aliyor_muyuz = false;

// !12:34:56 mantýðý için:
static char time_buf[12];
static uint8_t time_i = 0;
static bool time_mode = false;

// Fonksiyon Prototipleri
void AyarlariYap(void);
void Timer0_ISR(void);
void ADC_Oku(void);
void UART0_Init(void);
void UART0_SendLine(const char *s);

// =========================================================
// UART RX: HEM <XYZ> HEM DE !SAAT KOMUTLARINI ÝÞLER
// =========================================================
static void UART_RX_Process(void)
{
    while (UARTCharsAvail(UART0_BASE))
    {
        char c = UARTCharGet(UART0_BASE);
        UARTCharPut(UART0_BASE, c);   // DEBUG ECHO


        // -------------------------------------------------
        // 1. KISIM: <ABC> MANTIÐI (LCD ÜST SATIR)
        // -------------------------------------------------

        // Baþlangýç: '<'
        if (c == '<')
        {
            veri_aliyor_muyuz = true;
            lcd_idx = 0;
            continue; // Diðer kontrollere bakma, sonraki harfe geç
        }

        // Eðer veri alýyorsak (yani < gelmiþse)
        if (veri_aliyor_muyuz)
        {
            // Bitiþ: '>'
            if (c == '>')
            {
                Lcd_Goto(1, 13);
                Lcd_Puts("OK");
                veri_aliyor_muyuz = false;
                lcd_temp_buf[lcd_idx] = '\0'; // Stringi kapat

                Lcd_Goto(1, 1);
                Lcd_Puts("                "); // 16 boþluk (üst satýrý FULL siler)
                Lcd_Goto(1, 1);
                Lcd_Puts(lcd_temp_buf);


                continue;
            }

            // Karakteri kaydet (Max 3 tane)
            if (lcd_idx < 3)
            {
                lcd_temp_buf[lcd_idx++] = c;
            }
            continue; // Bu harf < > içine aitti, aþaðýya (saate) düþmesin
        }

        // -------------------------------------------------
        // 2. KISIM: !12:34:56 MANTIÐI (SAAT AYARI)
        // -------------------------------------------------

        // Baþlangýç: '!'
        if (c == '!')
        {
            time_mode = true;
            time_i = 0;
            continue;
        }

        // Eðer saat modundaysak (yani ! gelmiþse)
        if (time_mode)
        {
            // Satýr sonu geldi mi? (\r veya \n)
            if (c == '\r' || c == '\n')
            {
                time_buf[time_i] = '\0'; // Stringi kapat
                time_mode = false;       // Modu bitir

                int h, m, s;
                // Parse et
                if (sscanf(time_buf, "%d:%d:%d", &h, &m, &s) == 3)
                {
                    if (h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60)
                    {
                        saat   = (uint8_t)h;
                        dakika = (uint8_t)m;
                        saniye = (uint8_t)s;
                        ekran_guncelle = true; // LCD hemen güncellensin
                    }
                }
                continue;
            }

            // Karakter biriktir (Buffer taþmasýn diye kontrol et)
            if (time_i < sizeof(time_buf) - 1)
                time_buf[time_i++] = c;

            continue;
        }
    }
}
// DÝKKAT: Fonksiyon burada bitiyor. Parantez hatasý buradaydý.

// ===== MAIN =====
int main(void)
{
    AyarlariYap();

    Lcd_init();
    Lcd_Temizle();

    Lcd_Goto(1, 1);
    Lcd_Puts("                "); // 16 boþluk (üst satýrý FULL siler)
    Lcd_Goto(1, 1);
    Lcd_Puts(lcd_temp_buf);

    while (1)
    {
        // PC'den gelenleri iþle
        UART_RX_Process();

        // --- BUTON OKUMA ---
        if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0)
        {
            SysCtlDelay(SysCtlClockGet() / 3 / 100); // 10ms Debounce

            if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0)
            {
                UART0_SendLine("B1"); // Buton basýldý mesajý
                SysCtlDelay(SysCtlClockGet() / 3 / 2); // 0.5sn bekle (Spam engelle)
            }
        }

        // --- ADC OKUMA VE LCD GÜNCELLEME ---
        if (adc_oku_flag)
        {
            adc_oku_flag = false;
            ADC_Oku();

            // LCD Alt satýrýn sonuna ADC yaz
            sprintf(pot_buffer, "%4u", (unsigned int)adc_degeri);
            Lcd_Goto(2, 10);
            Lcd_Puts(pot_buffer);
        }

        // --- SAAT GÜNCELLEME ---
        if (ekran_guncelle)
        {
            ekran_guncelle = false;

            // LCD Alt satýrýn baþýna Saat yaz
            sprintf(saat_buffer, "%02u:%02u:%02u",
                    (unsigned int)saat, (unsigned int)dakika, (unsigned int)saniye);
            Lcd_Goto(2, 1);
            Lcd_Puts(saat_buffer);
        }

        // --- UART VERÝ GÖNDERÝMÝ ---
        if (uart_gonder)
        {
            uart_gonder = false;

            char gonder_buf[32];

            // ADC Gönder
            sprintf(gonder_buf, "*%u", (unsigned int)adc_degeri);
            UART0_SendLine(gonder_buf);

            // Saat Gönder
            sprintf(gonder_buf, "#%02u:%02u:%02u",
                    (unsigned int)saat, (unsigned int)dakika, (unsigned int)saniye);
            UART0_SendLine(gonder_buf);
        }
    }
}

// ===== Timer ISR =====
void Timer0_ISR(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    saniye++;
    if (saniye > 59) {
        saniye = 0;
        dakika++;
        if (dakika > 59) {
            dakika = 0;
            saat++;
            if (saat > 23) saat = 0;
        }
    }

    ekran_guncelle = true;
    adc_oku_flag   = true;

    sayac1++;
    if (sayac1 >= 1) // Orijinalinde 1'di
    {
        sayac1 = 0;
        uart_gonder = true;
    }
}

// ===== ADC Oku =====
void ADC_Oku(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false)) {}
    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, &adc_degeri);
}

// ===== UART Init =====
void UART0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {}

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);

    UARTEnable(UART0_BASE);
}

// ===== UART Satýr Gönder =====
void UART0_SendLine(const char *s)
{
    while(*s)
        UARTCharPut(UART0_BASE, *s++);

    UARTCharPut(UART0_BASE, '\r');
    UARTCharPut(UART0_BASE, '\n');
}

// ===== Ayarlar =====
void AyarlariYap(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    UART0_Init();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0_ISR);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);

    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);
}

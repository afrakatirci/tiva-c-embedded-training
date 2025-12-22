#include "stdint.h"
#include "stdbool.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"

#include "lcd.h"

// Zaman sayaçları
int saat = 0;
int dakika = 0;
int saniye = 0;

// Prototipler
void ZamanGoster(void);
void ZamanArttir(void);
void ADC_Init(void);
uint32_t ADC_Oku(void);
void SicaklikGoster(void);

// =======================
// LCD'de "hh:mm:ss" yaz
// =======================
void ZamanGoster(void)
{
    char zaman[17];

    // hh:mm:ss formatı
    zaman[0] = '0' + (saat / 10);
    zaman[1] = '0' + (saat % 10);
    zaman[2] = ':';
    zaman[3] = '0' + (dakika / 10);
    zaman[4] = '0' + (dakika % 10);
    zaman[5] = ':';
    zaman[6] = '0' + (saniye / 10);
    zaman[7] = '0' + (saniye % 10);

    // Satırın geri kalanını boşlukla doldur
    zaman[8]  = ' ';
    zaman[9]  = ' ';
    zaman[10] = ' ';
    zaman[11] = ' ';
    zaman[12] = ' ';
    zaman[13] = ' ';
    zaman[14] = ' ';
    zaman[15] = ' ';
    zaman[16] = '\0';

    // 2. satır, 1. sütun (sol alt)
    satir_sutun(2, 1);
    printf(zaman);
}

// =======================
// Zaman hesabı (00:00:00 → ...)
// =======================
void ZamanArttir(void)
{
    saniye++;
    if (saniye >= 60)
    {
        saniye = 0;
        dakika++;
        if (dakika >= 60)
        {
            dakika = 0;
            saat++;
            if (saat >= 24)
            {
                saat = 0;
            }
        }
    }
}

// =======================
// ADC Başlatma (PD3 / AIN4)
// =======================
void ADC_Init(void)
{
    // ADC0 ve GPIO D'yi aktif et
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // PD3 pinini ADC girişine çevir (AIN4)
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);

    // ADC0, sequencer 3, processor tetiklemeli
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Sıra: tek adım, kanal CH4 (AIN4), kesme ve end
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0,
                              ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 3);
}

// =======================
// ADC'den 1 örnek oku
// =======================
uint32_t ADC_Oku(void)
{
    uint32_t value;

    ADCIntClear(ADC0_BASE, 3);
    ADCProcessorTrigger(ADC0_BASE, 3);

    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    ADCSequenceDataGet(ADC0_BASE, 3, &value);

    return value; // 0–4095 arası (12 bit)
}

// =======================
// Sıcaklığı hesapla ve LCD'de göster
// 0–4095  →  0–40 °C olarak DÜZENLENDİ
// =======================
void SicaklikGoster(void)
{
    uint32_t adcDegeri = ADC_Oku();
    uint32_t sicaklik;    // 0–40 arası

    // Lineer ölçekleme: 0–4095 -> 0–40
    // Tamsayı bölme (Integer Division) kullanıldı.
    sicaklik = (adcDegeri * 40) / 4096;

    // Değerin 40'ı aşmamasını sağlamak için kontrol (isteğe bağlı)
    if (sicaklik > 40) {
        sicaklik = 40;
    }


    // " XXC" formatında yazalım (ör: " 25C")
    char buf[7];
    buf[0] = ' ';
    buf[1] = '0' + (sicaklik / 10);     // Onlar basamağı
    buf[2] = '0' + (sicaklik % 10);     // Birler basamağı
    buf[3] = 'C';
    buf[4] = ' ';
    buf[5] = ' ';
    buf[6] = '\0';

    // 2. satır, 11. sütun: zamanın sağ tarafında sıcaklık
    satir_sutun(2, 11);
    printf(buf);
}

// =======================
// main
// =======================
int main(void)
{
    // Sistem saatini ayarla (PLL, 16MHz kristal, 50MHz sistem clock)
    SysCtlClockSet(SYSCTL_SYSDIV_4 |
                   SYSCTL_USE_PLL    |
                   SYSCTL_OSC_MAIN   |
                   SYSCTL_XTAL_16MHZ);

    // LCD başlat
    baslangic();

    // ADC başlat
    ADC_Init();

    // LCD'ye sabit yazı: 1. satır sol üst "Afra Katirci"
    satir_sutun(1, 1);
    printf("Afra Katirci");

    // Başlangıç zamanı ve sıcaklığı göster
    ZamanGoster();
    SicaklikGoster();

    // Sonsuz döngü: her 1 saniyede zamanı arttır, LCD’yi güncelle
    while(1)
    {
        // 1 saniye ≈ SysCtlClockGet() / 3
        SysCtlDelay(SysCtlClockGet() / 3);

        ZamanArttir();
        ZamanGoster();
        SicaklikGoster(); // Sıcaklığı sürekli güncelle
    }
}


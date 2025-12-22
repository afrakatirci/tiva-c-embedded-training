#include "stdint.h"
#include "stdbool.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

#include "lcd.h"

// Zaman sayaçları
int saat   = 0;
int dakika = 00;
int saniye = 0;

// Prototipler
void ZamanGoster(void);
void ZamanArttir(void);

// =======================
// LCD'de "hh:mm:ss" yaz
// =======================
void ZamanGoster(void)
{
    char zaman[17];   // 16 karakter + '\0'

    // hh:mm:ss formatı
    zaman[0] = '0' + (saat / 10);
    zaman[1] = '0' + (saat % 10);
    zaman[2] = ':';
    zaman[3] = '0' + (dakika / 10);
    zaman[4] = '0' + (dakika % 10);
    zaman[5] = ':';
    zaman[6] = '0' + (saniye / 10);
    zaman[7] = '0' + (saniye % 10);

    // Satırın geri kalanını boşluk yap ki eski yazı kalıntısı kalmasın
    zaman[8]  = ' ';
    zaman[9]  = ' ';
    zaman[10] = ' ';
    zaman[11] = ' ';
    zaman[12] = ' ';
    zaman[13] = ' ';
    zaman[14] = ' ';
    zaman[15] = ' ';
    zaman[16] = '\0';

    // Sol alt köşe (2. satır, 1. sütun)
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
// main
// =======================
int main(void)
{
    // Sistem saatini ayarla (PLL, 16MHz kristal, 50MHz sistem clock)
    SysCtlClockSet(SYSCTL_SYSDIV_4 |
                   SYSCTL_USE_PLL   |
                   SYSCTL_OSC_MAIN  |
                   SYSCTL_XTAL_16MHZ);


    // LCD başlat
    baslangic();


    // LCD'ye sabit yazılar
    satir_sutun(1, 2);
    printf("Afra Katirci");

    satir_sutun(2, 1);
    printf("MIKRO LCD PROJE");


    // Başlangıç zamanı göster (00:00:00)
    ZamanGoster();

    // Sonsuz döngü: her 1 saniyede zamanı arttır ve LCD'yi güncelle
    while(1)
    {
        // SysCtlDelay: her çağrıda ~3 clock döngüsü sürer.
        // 1 saniye ≈ SysCtlClockGet() / 3
        SysCtlDelay(SysCtlClockGet() / 3);

        ZamanArttir();
        ZamanGoster();
    }
}

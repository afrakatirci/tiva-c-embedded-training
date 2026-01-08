#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "lcd.h"

int main(void)
{

    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); // Sistem saatini ayarlama
    // PLL kullanýlarak, ana osilatör ve 16 MHz kristal ile saat frekansý ayarlanýr
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);// GPIO Port F çevre birimini aktif hale getirir
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // Port F üzerindeki PF1, PF2 ve PF3 pinlerini çýkýþ olarak ayarlar
    // (Tiva kartýnda bu pinler genellikle RGB LED’lere baðlýdýr)
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4,  // Port F üzerindeki PF4 pinini giriþ olarak ayarlar
                     // (Genellikle kullanýcý butonu), GPIO_PIN_TYPE_STD_WPU);
    baslangic();
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);  // PF2 pinini HIGH yapar (mavi LED yanar)

    satir_sutun(1,2);// LCD imlecini 1. satýr 2. sütuna getirir
    printf("AFRAKATÝRCÝ");// LCD’ye "AFRA KATÝRCÝ" yazdýrýr
    satir_sutun(2,1); // LCD imlecini 2. satýr 1. sütuna getirir
    printf("MIKRO LCD PROJE");// LCD’ye "MIKRO LCD PROJE" yazdýrýr
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);// PF3 pinini HIGH yapar (yeþil LED yanar)
}


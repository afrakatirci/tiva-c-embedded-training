/*****************************************************************************
** 				SAKARYA ÜNÝVERSÝTESÝ	                    **
**   			  ELEKTRÝK ELEKTRONÝK MÜHENDÝSLÝÐÝ		    **
** 			  	   NECATÝ BÝLGÝN			    **
** 			  	    B090100043				    **
**		     4-BÝT VERÝ YOLLU LCD HEADER FÝLE DOSYASI  		    **
**		 	     	    20.05.2014		                    **
******************************************************************************
**          LCD Pinlerinin Stellaris LaunchPad Pinleri ile Baðlantýsý	    **
** 		   D7 - D6  - D5  - D4					    **
** Veri hatti ==> PB7 - PB6 - PB5 - PB4	   MSB:YUKSEK ANLAMLI BIT	    **
** 		  MSB	            LSB	   LSB:DUSUK ANLAMLI BIT	    **
**									    **
** RS biti ==> PE1							    **
** R/W biti ==> PE2						            **
** EN biti ==> PE3							    */
//===========================================================================
#ifndef _LCD_H
#define _LCD_H

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
// LCD kontrol pinleri (Port E)
#define RS	GPIO_PIN_1// Register Select: Komut / Veri seçimi
#define RW	GPIO_PIN_2 // Read/Write: Okuma / Yazma seçimi
#define EN	GPIO_PIN_3// Enable: LCD tetikleme pini

// LCD veri pinleri (4-bit mod, Port B)
#define D4	GPIO_PIN_4
#define D5	GPIO_PIN_5
#define D6	GPIO_PIN_6
#define D7	GPIO_PIN_7

long sure=50000;// LCD iþlemleri için gecikme süresi

// Fonksiyon prototipleri

void baslangic(void);
void komut_yaz(void);
void LCD_sil(void);
void veri_yaz(void);
void satir_sutun(unsigned char satir, unsigned char sutun);
char veri(char deger);
void printf( char* s);
//===========================================================================
void baslangic(void)   //LCD baþlangýç ayarlarýný yapar.
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);// Port B aktif
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);// Port E aktif


	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, RS | RW | EN);   // RS, RW, EN pinlerini çýkýþ yap
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, D7 | D6  | D5 | D4);// D4–D7 veri pinlerini çýkýþ yap

	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
	SysCtlDelay(sure);    //min 20ms bekle

	// Display ON, cursor OFF ayarlarý
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x20);
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x20);
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x80);
	komut_yaz();
	SysCtlDelay(sure);    // min 37us bekle
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0xD0);
	komut_yaz();
	SysCtlDelay(sure);    //min 37us bekle
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x10);
	komut_yaz();
	SysCtlDelay(sure);    //min 1.52ms bekle
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x20);
	komut_yaz();
}
//===========================================================================
void komut_yaz(void)// LCD’ye komut gönderme iþlemi (EN tetikleme)
{
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x08);// EN = 1
	SysCtlDelay(10000); // Kýsa gecikme
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00); // EN = 0
}
//===========================================================================
void satir_sutun(unsigned char satir, unsigned char sutun)// LCD’de imleci istenilen satýr ve sütuna götürür
{
	char total;
	if(satir == 1)// Satýr baþlangýç adresleri
		{satir = 0x7F;}
	if(satir == 2)
		{satir = 0x0BF;}
	total = satir + sutun; // DDRAM adresi hesaplanýr
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, (0xF0 & total));// Adresin yüksek 4 biti gönderilir
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, ((total & 0x0F) << 4));   // Adresin düþük 4 biti gönderilir
	komut_yaz();
}
//===========================================================================
void LCD_sil(void)   //LCD ekranýný siler.
{
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
	SysCtlDelay(sure);
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
	SysCtlDelay(sure);
	komut_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x10);
	SysCtlDelay(sure);
	komut_yaz();
}
//===========================================================================
void printf( char* s)// LCD’ye string (yazý) yazdýrýr
{
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x02);
	while(*s)
    veri(*s++);
}
//===========================================================================
char veri(char deger)// LCD’ye tek karakter gönderir (4-bit mod)
{
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, (0xF0 & deger)); //YÜKSEK 4 BÝT
	veri_yaz();
	GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, ((deger & 0x0F) << 4));//DÜÞÜK 4 BÝT
	veri_yaz();
}
//===========================================================================
void veri_yaz(void)// LCD’ye veri yazma tetikleme iþlemi
{
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x0A);
	SysCtlDelay(10000);
	GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x02);
}
#endif


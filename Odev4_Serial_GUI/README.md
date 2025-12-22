# Ödev 4 – Seri Port ve GUI Haberleşmesi
Bu proje, Tiva C serisi mikrodenetleyici ile bilgisayar arasında UART protokolü kullanılarak seri haberleşmenin gerçekleştirilmesini ve bu haberleşmenin bir grafik kullanıcı arayüzü (GUI) üzerinden kontrol edilmesini amaçlamaktadır.
Proje iki ana bölümden oluşmaktadır. İlk bölümde, Tiva C mikrodenetleyici üzerinde UART konfigürasyonu yapılarak veri gönderme ve alma işlemleri gerçekleştirilmiştir. İkinci bölümde ise C#, SharpDevelop kullanılarak geliştirilen GUI aracılığıyla seri port üzerinden mikroişlemci ile etkileşim sağlanmıştır.

Sharp Developer kısmında oluşturulan arayüzde portno kısmında com adresi girilerek port bağlantısı oluşturulmaktadır.
LCD'ye saat gönder kısmına saat girilerek saat verisi gönderilmektedir.
3.Kısımda 3 karakterli bir string ifade button 2 aracılığıyla gönderilmektedir. 
LCD'de yazılan saat bilgisi ve  adc verisi ekrana gönderilmektedir. 
Buton durumu tiva üzerindeki buton1'e basılıp basılmadığını gösteren bir kutucuktur.

Veri akışı: Bilgisayar ile Mikroişlemci arasındaki UART protokolüyle gerçekleştirmektedir. Bilgisayar üzerinden mirkoişlemciye gönderilen veriler daha önceki çalışmalardaki aynı mantığı kullanarak veriyi LCD'ye göndermektedir. Mikroişlemci içerisine tanımlanan fonksiyonlar aracılığıyla gelen veriler LCD'ye gönderilerek yazılmaktadır.

### DEVRE GÖRSELİ:
![Image](https://github.com/user-attachments/assets/cc052e3c-e6c5-498d-9f41-2ba8bff97b0b)
![Image](https://github.com/user-attachments/assets/7a75b945-fa10-45f8-a9c9-58dc80012499)
### PİN BAĞLANTILARI:
![Image](https://github.com/user-attachments/assets/2f0f3a7e-35d6-487e-bd00-edec9a923f0c)
![Image](https://github.com/user-attachments/assets/cba931c3-3f40-4aba-a075-9e8786bd465e)
![Image](https://github.com/user-attachments/assets/fcdbc1df-112b-4f02-b900-317fba9d5660)
![Image](https://github.com/user-attachments/assets/49aff21f-59e9-4de5-95e6-a6905c6a442d)
![Image](https://github.com/user-attachments/assets/695376de-ceb1-49f5-80c4-3168469b93de)

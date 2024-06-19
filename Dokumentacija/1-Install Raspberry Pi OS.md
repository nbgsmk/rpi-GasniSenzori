
1 Raspberry Pi Zero W install
=============================

1.1 Priprema SD kartice za rpi
-------------------------------
   
Install rpi-imager

  (win)
  https://downloads.raspberrypi.org/imager/imager_latest.exe
  
  (Linux)
  https://downloads.raspberrypi.org/imager/imager_latest_amd64.deb
  
  ili 
  
      sudo apt-get install rpi-imager
  

  Kad se pokrene:
  
  1 - Choose device -> Raspberry Pi Zero (ne birati "Zero 2W")
  
  2 - Operating system[^1][^2] -> 
  
        Raspberry Pi OS (other) -> 
        
            Raspberry PI OS Lite (32-bit) - no desktop (0.5GB) = **NE RADI wifi**, na razne nacine ne radi. Duga prica...
            
            Raspberry PI OS (Legacy 32-bit) Lite - no desktop (0.4GB) = **RADI**
            
  3 - Choose storage -> 
  
        naravno, sd kartica predvidjena za instalaciju Rpi. Pazite sta formatirate :-)

4 - Next

5 - Use OS customisation -> Would you like to apply OS customization settings?

  **OBAVEZNO** izabrati "edit settings"! Passwordi za korisnika i wifi su, naizgled, zapamceni (vide se 'tackice') ali pokazalo se da nisu tacni.
Proveriti i ukucati __ponovo__!!
Ovo treba uraditi **svaki put** kad se udje u "OS customizations" prozorcic i, uopste, za svako sledece pokretanje imagera.
Verovatno je u pitanju neka sigurnosna mera - ne cuvati unsecure kredencijale, ne znam. U svakom slucaju, kod mene tako radi.
Posto ce se realno vrlo retko reinstalirati linux na sd karticu, ovo nije neka smetnja.

  Dalje:

  Tab - GENERAL

  hostname -> po zelji npr zero.local

  username and password -> po zelji (**obavezno** obrisati tackice i ukucati ponovo!)

  wireless lan -> naziv mreze i password (isto kao i gore **obavezno** ukucati password ponovo)
  
  Klikni na "show password" i videces zasto

 
  Tab - SERVICES

  Ako nista drugo, makar za debugging, ukljuciti **SSH** -> password authentication
  

  Tab - OPTIONS

  Kako god hoces

  -> SAVE

  
Ponovo smo na prozoru Use OS customisation -> Would you like to apply OS customization settings?

Ovaj put izabrati **YES**

Potvrditi upozorenje da ce svi podaci na SD kartici biti obrisani. Pazljivo :-)

[^1]: Verzija "Lite (32-bit)" = Raspbian 12. NECE da startuje bez povezanog hdmi kabla!  Pitaju ljudi po forumima "can't start headless". Mozda da se resi za neku buducu verziju. Za sada nisam hteo dalje da trazim.  
Cak i ako se poveze hdmi, izgleda da 12-ica koristi "NetworkManager" sistem koji se ne slaze sa wpa_supplicant-om. Verovatno zato ne startuje wifi, tj wpa_supplicant naizgled radi ali, nakon sto skenira wifi mreze, **ne moze da snimi konfiguraciju.** Nisam hteo dalje da se zamajavam sa time.

[^2]: Verzija "(Legacy 32-bit) Lite" = Raspbian 11. Mogli su to lepse da napisu. :-) Nakon prve instalacije, rpi uradi jedan dodatni reboot. Nesto priprema sd karticu, ovo ono... 40 sekundi do prve poruke na terminalu ali 3-4 minuta dok ne poveze wifi! Sledeci restarti idu daleko brze.  
Startuje bez hdmi, nema greske!  
Startuje sa hdmi, nema greske!  


2 Serijska konzola na rpi
-------------------------

Ako unapred znas da ce biti potreban debugging, moze se na gpio header povezati sistemska serijska konzola:

- Ako je kartica i dalje u racunaru na kojem je obavljen rpi-imager

        (Potrebna nam je **/boot** particija. Pronaci gde je mountovana kartica, npr kod mene je /media/zoki/boot)
        $ sudo nano /media/zoki/boot/config.txt
        -> na kraj fajla dodati
        enable_uart=1
      
- Ako je kartica vec prebacena u raspberry i on je startovan

      Direktno iz terminala na rpi:
      $ sudo nano /etc/boot/config.txt
      -> na kraj fajla dodati
      enable_uart=1
      -> restartovati rpi

Nakon ovoga, sistemska konzola je povezana na gpio header (fizicki pinovi 8 i 10). ALI, ovo rezervise serijski port za sistemsku konzolu linuxa i drugi uredjaji ne mogu da komuniciraju. **Rpi zero NEMA druge slobodne serijske portove.** Kad se osnovne stvari srede, i bude moguce da se radi preko ssh i preko mreze, ovo svakako mora da se iskljuci.


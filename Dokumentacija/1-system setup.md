
Raspberry Pi Zero W install
===========================

1) Priprema SD kartice za rpi
   --------------------------
   
Install rpi-imager

  (win)
  https://downloads.raspberrypi.org/imager/imager_latest.exe
  
  (Linux)
  https://downloads.raspberrypi.org/imager/imager_latest_amd64.deb
  
  ili 
  
      sudo apt-get install rpi-imager
  

  Kad se pokrene:
  
  1 - Choose device -> Raspberry Pi Zero (ne birati "Zero 2W")
  
  2 - Operating system -> 
  
        Raspberry Pi OS (other) -> 
        
            Raspberry PI OS Lite (32-bit) - no desktop (0.5GB) = **NE RADI wifi**, na razne nacine ne radi. Duga prica...
            
            Raspberry PI OS (Legacy 32-bit) Lite - no desktop (0.4GB) = **RADI**
            
  3 - Choose storage -> 
  
        naravno, sd kartica predvidjena za instalaciju Rpi. Pazite sta formatirate :-)

4 - Next

5 - Use OS customisation -> Would you like to apply OS customization settings?

  **OBAVEZNO** izabrati "edit settings"! Passwordi za korisnika i wifi su, naizgled, zapamceni (vide se 'tackice') ali pokazalo se da nisu tacni.
  Proveriti i ukucati ponovo!!
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

  Kako god hocete

  -> SAVE

  
Ponovo smo na prozoru Use OS customisation -> Would you like to apply OS customization settings?

Ovaj put izabrati **YES**

Potvrditi upozorenje da ce svi podaci na SD kartici biti obrisani. Pazljivo :-)




  

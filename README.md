# SpieleKnecht
Code & Dateien für das Projekt SpieleKnecht, einem Microcontroller basierten
System für Rundenbasierte mehrspieler Spiele. Als Beispielprojekt ist TicTacToe
programmiert, das System Spieleknecht kann allerdings flexibel und mit relative wenig Aufwand 
für andere Spiele Adaptiert werden.
SpieleKnecht entstand im Rahmen der Projektphase des BGY21b-Mikrokontroller Kurs
in Kooperation zwischen David, Jakob, Simon und Vincent unter Aufsicht und Unterstützung von Herr Braunschädel.

## Hardware
Das Projekt basiert auf dem ESP8266 in Form eines Witty Boards als Bildschirm dient ein SSD1306 OLED. Das Pinout ist
(up, down, left, right, a, b, sck, sda, speaker) zu (D7, RX, D6, D5, D4, D3, D1, D2, D8) in der gleichen Reihenfolge.
(up, down, left, right, a, b) sind Active-Low Taster.

![image](https://github.com/testmonkeyontherun/esp-tictactoe-server/blob/main/Schaltung.png)
![image](https://github.com/testmonkeyontherun/esp-tictactoe-server/blob/main/SchaltplanZeichnung.png)


## Installation & Nutzung
### ESP Client:
Der [ESP8266](https://github.com/esp8266/Arduino) muss erst in der Arduino-IDE installiert werden.
Danach müssend folgende Bibliotheken aus dem Bibliothekenverwalter installiert werden.
- ArduinoJson
- Adafruit GFX
- Adafruit SSD1306

Wenn das board auf "LOLIN(WEMOS) D1 R2 & mini" steht kann das Programm kompiliert und ausgeführt werden.
Nach jedem Durchlauf muss der ESP durch Enfernen und Hinzufügen von Strom zurückgesetzt werden.
### Python Server & Client
Der Python-Server und Python-Client brauchen python 3.11 oder höher.
### Nutzung
Der server wird durch
```console
$ python server.py
```
gestartet. Der ESP verbindet sich nach dem einschalten mit dem in esp_client/server.h konfigurierten WiFi und Server. Wenn nur ein ESP zur verfügung steht kann ein weiterer client über
```console
$ python client.py
```
gestartet werden.

# etrodiag
RUS: Это приложение предназначено для парсинга кастомного протокола обмена информацией между сетью микроконтроллеров. 
Протокол представляет из себя поочерёдную трансляцию пакетов размером 40 байт, где 40 и 39 байты это маркер начала пакета (FF), 38 байт - номер микроконтроллера в сети.
Последовательности байт можно определять в слова размером 8, 16 или 32 бит.
Из этих слов нужные данные получаются при помощи масок.
Данные из масок отображаются в окне лога с таймштампом, что позволяет представлять обмен информацией в человеческом виде и соотносить отображаемые события с реальным поведением устройств.
Реализовано логирование обработанных данных, сохранение профилей для различающихся сетей устройств, подсветка обновляемых данных при настройке профиля и индикация потери связи с устройством.

Это первый мой проект на QT, реализуя его я изучаю возможности языка. Добро пожаловать в мир костылей и велосипедов.

EN (Google translate): This application is intended for parsing a custom protocol for the exchange of information between a network of microcontrollers.
The protocol is an alternate broadcast of packets of 40 bytes in size, where 40 and 39 bytes are the packet start marker (FF), 38 bytes are the number of the microcontroller on the network.
Byte sequences can be defined in words of size 8, 16 or 32 bits.
From these words, the necessary data is obtained using masks.
Data from the masks is displayed in the log window with a time stamp, which allows you to represent the exchange of information in human form and correlate the displayed events with the actual behavior of the devices.
Implemented logging of processed data, saving profiles for different device networks, highlighting updated data when setting up a profile, and indicating loss of connection with the device.

This is my first project on QT, implementing it I study the possibilities of the language. Welcome to the world of crutches and bicycles.

# etrodiag
RUS: Это приложение предназначено для парсинга кастомного протокола обмена информацией микроконтроллерами на общей линии данных, общающихся между собой по принципу master-slave по кругу.  
Протокол представляет из себя поочерёдную трансляцию пакетов размером 40 байт, где 40 и 39 байты это маркер начала пакета (FF), 38 байт - номер микроконтроллера в сети.
Последовательности байт можно определять в слова размером 8, 16 или 32 бит.
Из этих слов нужные данные получаются при помощи масок.
Данные из масок отображаются в окне лога с таймштампом, что позволяет представлять обмен информацией в человеческом виде и соотносить отображаемые события с реальным поведением устройств.
Реализовано логирование обработанных данных, сохранение профилей для различающихся сетей устройств, подсветка обновляемых данных при настройке профиля, рисование живого графика с выбранными параметрами и индикация потери связи с устройством.

EN (Google translate): This application is designed for parsing a custom protocol for the exchange of information by microcontrollers on a common data line, communicating with each other according to the master-slave principle in a circle.
The protocol is a sequential broadcast of 40-byte packets, where 40 and 39 bytes are the packet start marker (FF), 38 bytes are the microcontroller number in the network.
Byte sequences can be specified in words of 8, 16, or 32 bits.
From these words, the necessary data is obtained using masks.
The data from the masks are displayed in a log window with a time stamp, which allows you to represent the exchange of information in a human form and correlate the displayed events with the real behavior of devices.
Logging of processed data, saving profiles for different networks of devices, highlighting updated data when configuring a profile, drawing a live graph with selected parameters and indication of loss of connection with a device are implemented.

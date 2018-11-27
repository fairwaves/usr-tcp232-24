USR - настройка преобразователей интерфейсов USR-TCP232-24
==========================================================

USR-TCP232-24 - это преобразователь интерфейсов, выпускаемый китайской фирмой Jinan USR IOT Technology Limited. Он позволяет подключать к сети Ethernet устройства с интерфейсами RS-232 и RS-485. Сам преобразователь представляет собой печатную плату, защищённую термоусадочной оболочкой. Питается преобразователь интерфейсов от блока питания на 5 Вольт.

Нельзя сказать, что устройство дешёвое, но при ближайшем рассмотрении можно понять, что разработчики старались максимально удешевить устройство: у него нет не только корпуса, но и сколь-нибудь общепринятого интерфейса. Устройство не поддерживает ни telnet, ни SNMP, ни HTTP. Для его настройки используется программа для Windows, которая взаимодействует с устройство по фирменному протоколу, состоящему из UDP-пакетов двух типов, которые всегда шлются на широковещательный адрес 255.255.255.255 и на порт 1500.

Пакеты первого типа отправляются самим устройством в ответ на запрос обнаружения. Они содержат в себе информацию о MAC-адресе преобразователя интерфейсов и его текущих настройках.

Пакеты второго типа в сторону устройства отправляет программа настройки. В этих пакетах содержится MAC-адрес настраиваемого устройства и его новые настройки.

Программа для настройки преобразователей интерфейсов есть только в варианте для Windows. Поскольку я пользуюсь Linux, мне хотелось иметь программу для Linux, чтобы не прибегать к помощи wine или виртуальных машин с Windows.

К счастью, описание протокола можно лего найти в интернете. Сопоставить описание протокола с реальностью можно при помощи сетевого снифера, прослушивая трафик между преобразователем интерфейсов и программой для Windows, запущенной в виртуальной машине.

Этот преобразователь интерфейсов понадобился мне для доступа к электросчётчику Энергомера CE102M-R5 145 A, который я приобрёл в новую квартиру. Последняя буква A в модели счётчика обозначает, что счётчик оснащён интерфейсом RS-485. На работе приходилось иметь дело и с такими преобразователями интерфейсов и с электросчётчиками, но разобраться во всём подробнее на работе нет возможности - в капиталистической системе важнее занять поляну быстрее конкурентов, а не достичь более качественного результата.

Кроме счётчика и преобразователя интерфейсов я также купил блок питания для установки на DIN-рейку рядом со счётчиком. К сожалению, в ящике с электросчётчиком для этого блока питания не хватило места, поэтому он переехал в соседний ящик, поближе к преобразователю интерфейсов, где особого смысла в таком блоке питания уж не было.

Использование программы
-----------------------

Для сборки программы из исходных текстов можно воспользоваться имеющимся в комплекте файлом make.sh

Если запустить программу из командной строки, не указывая ей никаких аргументов, программа выведет справку по использованию, которая выглядит следующим образом:

```
Usage: usr discover <interface>
       usr config|change <interface> <mac> <options>
Options:
  Serial interface:
    -1               - use one stop bit
    -2               - use two stop bits
    -5               - use five data bits
    -6               - use six data bits
    -7               - use seven data bits
    -8               - use eight data bits
    -N               - don't use parity bit
    -E               - use even parity bit control
    -O               - use odd parity bit control
    -M               - always set parity bit
    -C               - always clear parity bit
    -r <bitrate>     - use specified bitrate, bits/s
    -w <word-format> - use specified data bits, parity control mode
                       and stop bits
                       Example: 8N1 - 8 data bits, no parity control, 1 stop bit
    -s <serial>      - use specified bitrate, data bits, parity control mode
                       and stop bits. Examples: 9600/8N1, 19200/7-E-1
  Ethernet interface:
    -i <ip>          - use specified IP
    -n <netmask>     - use specified subnetwork mask
    -g <gateway>     - use specified gateway
  Converter mode:
    -u               - use UDP-client mode
    -U               - use UDP-server mode
    -t               - use TCP-client mode
    -T               - use TCP-server mode
    -m               - use specified source port
                       (for UDP-client and TCP-client modes)
                       or listen specified port
                       (for UDP-server and TCP-server modes)
    -d <dest-ip>     - use specified destination ip
                       (for UDP-client and TCP-client modes)
    -p <dest-port>   - use specified destination port
                       (for UDP-client and TCP-client modes)
  Additional settings:
    -I <id>          - use specified converter identifier
                       (only in TCP-client mode and enabled flags
                       connect-id or/and data-id)
    -f <flags>       - use only specified comma-separated flags
    -f +<flags>      - enable specified comma-separated flags
    -f -<flags>      - disable specified comma-separated flags
  Flags:
    connect-id       - send converter identifier after connection was
                       established
    data-id          - send data with converter identifier
    rs485            - use interface RS-485 instead of RS-232
    rs422            - use interface RS-422 instead of RS-232
    reset            - reset converter after 30 failed connections to server
                       (only in TCP-client mode)
    link             - enable link led when connection is established
    index            - enable indexing of incoming connections
                       (only for TCP-server mode)
    rfc2217          - enable RFC2217 functions for temporarily changing of
                       serial settings (only before next restart of converter)
```

Как можно увидеть, программа может работать в одном из трёх режимов:
* `discover <interface>` - режим обнаружения преобразователей интерфейсов, находящихся в локальной сети за указанным сетевым Ethernet-интерфейсом,
* `config <interface> <mac> <options>` - режим настройки преобразователя интерфейсов с указанным MAC-адресом и за указанным сетевым Ethernet-интерфейсом. Опции указывают настройки, которые нужно выставить на указанном преобразователе интерфейсов. Все не указанные настройки примут значение по умолчанию,
* `change <interface> <mac> <options>` - режим изменения настроек преобразователя интерфейсов с указанным MAC-адресом и за указанным сетевым Ethernet-интерфейсом. Опции указывают новые значения настроек, которые нужно поменять. Все не указанные настройки сохраняют свои прежние значения.

Опции для настройки преобразователя интерфейсов собраны в логически взаимосвязанные группы:

### Serial interface - настройки последовательного интерфейса

### Ethernet interface - настройки сетевого Ethernet-интерфейса

### Converter mode - настройки режима работы преобразователя интерфейсов

### Additional settings - дополнительные настройки, в том числе - флажки

### Flags - флажки

Лицензия
--------

(C) 2018 Владимир Ступин

Программа распространяется под лицензией GPL 3.

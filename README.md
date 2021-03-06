USR - настройка преобразователей интерфейсов USR-TCP232-24
==========================================================

USR-TCP232-24 - это преобразователь интерфейсов, выпускаемый китайской фирмой Jinan USR IOT Technology Limited. Он позволяет подключать к сети Ethernet устройства с интерфейсами RS-232, RS-422 и RS-485. На плате имеется два физических интерфейса: первый может работать как интерфейс RS-232 или RS-422, а второй предназначен для подключения только устройств с интерфейсом RS-485. Одновременно может использоваться только один из последовательных интерфейсов. Сам преобразователь представляет собой печатную плату, защищённую термоусадочной оболочкой. Питается преобразователь интерфейсов от блока питания на 5 Вольт. В России его часто можно встретить под названием SNR-RS485-Ethernet.

Мне встречались телефонные станции, которые умели слать на порт RS-232 информацию о звонках. Это устройство можно использовать для отправки этих данных на компьютер с Ethernet-интерфейсом.

Существует масса промышленных устройств, работающих по интерфейсу RS-485. В отличие от интерфейса RS-232, интерфейс RS-485 обладает более высокой помехоустойчивостью, позволяет обмениваться данными на скорости до 10 мегабит в секунду и подключать к одному ведущему устройству до 10 ведомых устройств. Например, это могут быть электросчётчики.

Нельзя сказать, что устройство дешёвое, но при ближайшем рассмотрении можно понять, что разработчики старались максимально удешевить устройство: у него нет не только корпуса, но и сколь-нибудь общепринятого интерфейса. Устройство не поддерживает ни telnet, ни SNMP, ни HTTP. Для его настройки используется программа для Windows, которая взаимодействует с устройством по фирменному протоколу, состоящему из UDP-пакетов двух типов, которые всегда шлются на широковещательный адрес 255.255.255.255 и на порт 1500.

Пакеты первого типа отправляются самим устройством в ответ на запрос обнаружения. Они содержат в себе информацию о MAC-адресе преобразователя интерфейсов и его текущих настройках.

Пакеты второго типа в сторону устройства отправляет программа настройки. В этих пакетах содержится MAC-адрес настраиваемого устройства и его новые настройки.

Программа для настройки преобразователей интерфейсов есть только в варианте для Windows. Поскольку я пользуюсь Linux, мне хотелось иметь программу для Linux, чтобы не прибегать к помощи wine или виртуальных машин с Windows.

К счастью, описание протокола можно легко найти в интернете. Сопоставить описание протокола с реальностью можно при помощи сетевого сниффера, прослушивая трафик между преобразователем интерфейсов и программой для Windows, запущенной в виртуальной машине.

Преобразователь интерфейсов я купил его в интернет-магазине [E2E4](https://e2e4online.ru). Этот преобразователь интерфейсов понадобился мне для доступа к электросчётчику Энергомера CE102M-R5 145 A, который я приобрёл в новую квартиру. Последняя буква A в модели счётчика обозначает, что счётчик оснащён интерфейсом RS-485. На работе приходилось иметь дело и с такими преобразователями интерфейсов и с электросчётчиками, но разобраться во всём подробнее на работе нет возможности - в капиталистической системе важнее занять поляну быстрее конкурентов, а не достичь более качественного результата.

Кроме счётчика и преобразователя интерфейсов я также купил блок питания для установки на DIN-рейку рядом со счётчиком. К сожалению, в ящике с электросчётчиком для этого блока питания не хватило места, поэтому он переехал в соседний ящик, поближе к преобразователю интерфейсов, где особого смысла в таком блоке питания уж не было.

Особенности реализации
----------------------

Реализованы режимы обнаружения, прописывания настроек и смены настроек.
1. В режиме обнаружения через указанный интерфейс отправляется пакет обнаружения, после чего в течение 5 секунд принимаются широковещательные UDP-пакеты с данными откликнувшихся устройств. Для гарантированного завершения обнаружения в отведённое время в 5 секунд используется системный интервальный таймер и перехват сигнала SIGALRM. Для отправки широковещательных пакетов используется специальная опция сетевого сокета SO_BROADCAST, разрешающая отправку пакетов на широковещательный адрес. Для привязки сокета к сетевому интерфейсу с указанным именем используется опция сетевого сокета SO_BINDTODEVICE.
2. В режиме прописывания настроек выполняется отправка пакета с новыми настройками преобразователя интерфейса. Настройки конструируются из значений по умолчанию и заменяющих их значений, указанных в опциях командной строки. После прописывания настроек выдерживается пауза для применения настроек преобразователем интерфейсов, после чего выполняется обнаружение устройства с указанным MAC-адресом. Отправленные настройки сверяются с обнаруженными. При успешном применении настроек выводится соответствующее сообщение.
3. В режиме смены настроек сначала выполняется обнаружение устройства с указанным MAC-адресом. Обнаруженные настройки берутся за основу, а изменяемые значения берутся из опций командной строки. Дальнейшие действия аналогичны прописыванию настроек: после прописывания настроек выдерживается пауза для применения настроек преобразователем интерфейсов, после чего выполняется обнаружение устройства с указанным MAC-адресом. Отправленные настройки сверяются с обнаруженными. При успешном применении настроек выводится соответствующее сообщение.

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
    -C, -S           - always clear parity bit
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
    -m <port>        - use specified source port
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

При обнаружении преобразователей интерфейсов результаты будут выводиться примерно в таком виде:

```
MAC:     00:14:54:CF:5C:41
Version: 5.8
Mode:    TCP-server
Flags:
         Use RS485
IP:      169.254.254.7
Netmask: 255.255.255.0
Gateway: 169.254.254.1
Port:    20108
Serial:  19200/7-E-1
```

Режимы `config` и `change` распознают MAC-адрес в любом из распространённых форматов:
* `00:11:22:33:44:55`,
* `00-11-22-33-44-55`,
* `0011:2233:4455`,
* `0011-2233-4455`,
* `001122:334455`,
* `001122-334455`,
* `00112233445566`,
* `0:1:2:33:44:55`.

Опции для настройки преобразователя интерфейсов собраны в логически взаимосвязанные группы:

### Serial interface - настройки последовательного интерфейса

Последовательные интерфейсы могут работать в одном из двух режимов:
* Синхронном, когда данные передаются с постоянной скоростью, без пауз,
* Асинхронном, когда данные передаются по мере необходимости.

При асинхронном режиме передаче данных возникает необходимость согласования обеими сторонами моментов начала и прекращения обмена данными. Существует два метода управления потоком данных:
* Аппаратный, когда для согласования обмена данными используются выделенные сигнальные линии,
* Программный, когда в потоке передаваемых данных передаются особые последовательности данных, обозначающие начало и конец передачи. Сами передаваемые данные при этом кодируются таким образом, чтобы в них эти особые последовательности не встречались.

При программном методе управления потоком каждая передаваемая порция данных начинается с одного единичного старт-бита, а заканчивается одним или более стоп-битами. Непосредственно после порции данных, но до стоповых битов, может также передаваться бит, используемый для контроля правильности передаваемых данных - бит контроля чётности. Если бит контроля чётности имеется, то он должен принимать такое значение, чтобы вместе с битами данных единичных битов было либо чётное количество (контроль чётности - Even), либо чтобы вместе с битами данных единичных битов было нечётное количество (контроль нечётности - Odd). Ещё существует возможность вставлять бит контроля чётности, но не использовать его и выставлять либо всегда равным единице (Mark), либо всегда равным нулю (Space или Clear).

Преобразователь интерфейсов поддерживает только асинхронный режим с программным методом управления потоком.

Для настройки количества битов данных предназначены опции `-5`, `-6`, `-7` и `-8`. Каждая из этих опций настраивает указанное количество бит данных.

Для настройки количества стоп-битов предназначены опции `-1` и `-2`, которые настраивают один или два стоп-бита соответственно.

Для настройки бита контроля чётности предназначены следующие опции:
* `-N` - нет бита чётности,
* `-E` - контроль чётности,
* `-O` - контроль нечётности,
* `-M` - всегда выставлять бит контроля чётности в единичное значение,
* `-C` или `-S` - всегда выставлять бит контроля чётности в нулевое значение.

Все эти настройки вместе полностью определяют формат слова, передаваемого по последовательной линии. Формат слова можно указать, не прибегая к указанным выше опциям, а воспользовавшись опцией `-w <word-format>`. Программа принимает формат слова, указанный одной строчкой. Например: 8N1, 7-E-1, 7-C-1, 6S2, 5M2.

Скорость передачи данных в битах в секунду можно указать при помощи опции `-r <bitrate>`. Стандартные значения битовой скорости таковы: 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600. Программа принимает только значения, указанные в этом списке, а при указании других значений будет сообщать об ошибке.

При помощи опции `-s <serial>` можно указать одновременно и формат слова и скорость передачи данных по последовательному интерфейсу. Значения этой опции могут быть, например, такими: 9600/8N1, 19200/7-E-1. Таким образом, одна эта опция может заменить все предыдущие опции, рассмотренные выше.

### Ethernet interface - настройки сетевого Ethernet-интерфейса

В этой группе настроек есть всего три опции:
* `-i <ip>` - настройка IP-адреса преобразователя интерфейсов,
* `-n <netmask>` - настройка маски подсети, при помощи которой указываются локальные IP-адреса, к которым можно отправлять пакеты напрямую, находя соответствующие им MAC-адреса через ARP-запросы,
* `-g <gateway>` - настройка шлюза по умолчанию. Шлюз по умолчанию представляет собой IP-адрес, на который будут отправляться пакеты для IP-адресов вне локальной сети. В качестве MAC-адреса получателя Ethernet пакета будет выставлен MAC-адрес шлюза по умолчанию, найденный через ARP-запрос. При этом в заголовке IP-пакета, вложенного в Ethernet-кадр, будет указан реальный IP-адрес получателя. Шлюз получит Ethernet-кадр, адресованный ему, заглянет в заголовок IP-пакета и определит, куда нужно передать пакет дальше.

К сожалению, в устройстве не предусмотрена поддержка протокола IPv6. Скорее всего из-за всё того же стремления максимально удешевить устройство.

### Converter mode - настройки режима работы преобразователя интерфейсов

В этой группе настроек можно указать режим работы преобразователя интерфейсов. Преобразователь интерфейсов может выступать в одной из двух ролей:
* в роли сервера, ожидая поступления входящего подключения или пакетов на порт с указанным номером,
* в роли клиента, подключаясь на указанные IP-адрес и порт при поступлении данных на последовательный интерфейс.

Как в той, так и в другой роли преобразователь интерфейсов может работать по одному из двух транспортных протоколов: UDP или TCP.

Роль и протокол настраиваются при помощи одной из следующих опций:
* `-u` - UDP-клиент,
* `-U` - UDP-сервер,
* `-t` - TCP-клиент,
* `-T` - TCP-сервер.

При помощи опции `-m <port>` можно указать порт, который будет прослушиваться преобразователем интерфейсов, работающим в роли сервера, или будет использоваться в качестве исходящего при работе преобразователя интерфейсов в роли клиента.

Если пребразователь будет работать в режиме клиента, то при помощи опций `-d <dest-ip>` и `-p <dest-port>` можно указать IP-адрес и порт удалённого узла, в направлении которого будут предприниматься попытки установить TCP-соединение или отправить UDP-пакеты.

### Additional settings - дополнительные настройки, в том числе - флажки

Если несколько преобразователей интерфейсов настроены в режиме TCP-клиента и подключаются на один и тот же IP-адрес и порт, то компьютер, принимающий подключения, может различить их по IP-адресу или порту, с которого установлено подключение. Однако может быть более удобным, если каждый подключающийся преобразователь интерфейсов будет отправлять числовой идентификатор при подключении или с каждой порцией данных. Значение этого идентификатора можно настроить при помощи опции `-I <id>`. Идентификатор может принимать значения от 0 до 65535. Учтите, что идентификатор будет использоваться преобразователем интерфейсов только в том случае, если на нём настроен соответствующий флаг - `connect-id` и/или `data-id` (см. ниже в разделе Flags - флажки).

Также у преобразователя интерфейсов имеется ряд других настроек, которые могут быть включены или выключены. Эти настройки называются флажками и могут изменяться при помощи опции `-f`:
* `-f <flags>` - установить только указанные флажки, остальные сбросить,
* `-f +<flags>` - установить только указанные флажки, остальные не менять,
* `-f -<flags>` - сбросить только указанные флажки, остальные не менять.

Все описанные выше опции программы могут быть указаны в командной строке многократно. При этом более поздние значения заменяют ранее указанные. В случае с опцией `-f` можно, например, указать её один (или более) раз со знаком `+` перед списком флажков, а один (или более) раз со знаком `-` перед списком флажков. Результат будет куммулятивным: флажки будут меняться в том порядке, в каком это предписывают сделать соответствующие опции `-f`, так что одни опции могут включить флажки, другие - отключить. Список флажков в аргументе опции flags разделяется запятыми. Знак `+` или `-` может быть указан только в начале списка флажков, не допускается указывать его перед каждым флажком индивидуально.

### Flags - флажки

Флажки:
* `connect-id` - после установки TCP-подключения со стороны преобразователя интерфейсов отправляеть числовой идентификатор преобразователя интерфейсов, настроенный опцией `-I <id>`,
* `data-id` - при передаче очередной порции данных, полученной с последовательного интерфейса, отправлять числовой идентификатор преобразователя интерфейсов, настроенный опцией `-I <id>`,
* `rs485` - использовать интерфейс RS-485, а не RS-232,
* `rs422` - использовать интерфейс RS-422, а не RS-232 (RS-422 - это более помехоустойчивый вариант интерфейса RS-232, в котором нули и единицы кодируются не уровнем напряжения, а разницей напряжений на линиях),
* `reset` - сбросить преобразователь интерфейсов после 30 неудачных попыток подключения (флажок используется только когда преобразователь интерфейсов настроен на работу в режиме TCP-клиента),
* `link` - включить светодиод, если подключение установлено,
* `index`- включить нумерацию входящих подключений (флажок используется только в режиме TCP-сервера и позволяет устройству с последовательным интерфейсом отличать друг от друга несколько входящих TCP-подключений),
* `rfc2217` - включить функции для временной смены настроек последовательного интерфейса в соответствии с RFC2217 (со стороны Ethernet-интерфейса можно поменять настройки скорости и формата слова последовательного интерфейса, которые будут отменены при разрыве или закрытии подключения).

Примеры
-------

Пример команды обнаружения преобразователей интерфейсов и результатов обнаружения:

```
# ./usr discover eth0
MAC:     00:14:54:CF:5C:41
Version: 5.8
Mode:    TCP-server
Flags:
         Use RS485
         Enable switching bitrate RFC2217
IP:      192.168.0.2
Netmask: 255.255.255.0
Gateway: 192.168.0.1
Port:    20108
Serial:  19200/7-E-1
```

Для настройки своего преобразователя интерфейсов для работы с электросчётчиком Энергомера CE102M-R5 145 A, я воспользовался вот такой командой:

`# ./usr config eth0 00-14-54-CF-5C-41 -s 9600/7E1 -i 169.254.254.7 -n 255.255.255.0 -g 169.254.254.1 -T -m 50 -f rs485`

Лицензия
--------

(C) 2018 Владимир Ступин

Программа распространяется под лицензией GPL 3.

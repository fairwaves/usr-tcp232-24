#ifndef __PROTO__
#define __PROTO__

#include <unistd.h>
#include "types.h"

/* Режимы работы преобразователя интерфейсов */
typedef enum conv_mode_e
{
  UDP_CLIENT,
  TCP_CLIENT,
  UDP_SERVER,
  TCP_SERVER
} conv_mode_t;

#define PARAMS_FLAGS_CONNECT 0x01 /* Отправлять идентификатор при подключении */
#define PARAMS_FLAGS_DATA    0x02 /* Отправлять данные вместе с идентификатором устройства */
#define PARAMS_FLAGS_RS485   0x04 /* Использовать интерфейс RS485 */
#define PARAMS_FLAGS_RS422   0x08 /* Использовать интерфейс RS422 */
#define PARAMS_FLAGS_RESET   0x10 /* В режиме TCP-клиента после 30 неудачных попыток
                                     осуществлять сброс устройства */
#define PARAMS_FLAGS_LINK    0x20 /* Включить светодиод, если подключение установлено успешно */
#define PARAMS_FLAGS_INDEX   0x40 /* Индекс устройства в режиме TCP-сервера,
				     см. пункт 4.12 в руководстве */
#define PARAMS_FLAGS_RFC2217 0x80 /* Смена битрейта при установленном подключении,
                                     см. пункт 4.13 в руководстве */

/* Настройки преобразователя интерфейсов */
typedef struct params_s
{
  /* Режим работы преобразователя интерфейсов */
  conv_mode_t mode;

  /* Настройки для режимов UDP_CLIENT и TCP_CLIENT: адрес и порт, куда отправлять данные */
  ip_t dest_ip;
  port_t dest_port;
  /* Дополнительные настройки */
  unsigned char flags;
  /* Опциональный идентификатор преобразователя интерфейсов для режима TCP_CLIENT 0..65535 */
  unsigned short id;

  /* Настройки интерфейса Ethernet */
  ip_t ip;
  ip_t netmask;
  ip_t gateway;
  port_t port;

  /* Настройки последовательного интерфейса */
  bitrate_t bitrate;
  unsigned char data_bits; /* 5..8 */
  unsigned char stop_bits; /* 1..2 */
  parity_t parity;
} params_t;

/* Значения параметров по умолчанию. Используются при сбросе настроек */
static const params_t DEFAULT_PARAMS = {
  TCP_CLIENT,
  {192, 168, 0, 201},
  8234,
  132,
  0,
  {192, 168, 0, 7},
  {255, 255, 255, 0},
  {192, 168, 0, 201},
  20108,
  115200,
  8,
  PARITY_DISABLED,
  1
};

/* Распаковка настроек преобразователя интерфейсов из пакета */
bool_t params_unpack(params_t *params, void *data, size_t size);

/* Упаковка настроек преобразователя интерфейсов в пакет */
ssize_t params_pack(params_t *params, void *data, size_t size);

/* Вывод настроек преобразователя интерфейсов */
bool_t params_print(params_t *params);

/* Возвращает TRUE, если значения всех полей двух параметров совпадают */
bool_t params_equal(params_t *a, params_t *b);

/* Пакет запроса на обнаружение преобразователей интерфейсов */
static const char DISCOVER_REQUEST[] = {
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
};

/* Структура данных, соответствующая ответу на обнаружение преобразователей интерфейсов */
typedef struct discover_s
{
  mac_t mac;
  unsigned char major_version;
  unsigned char minor_version;
  params_t params;
} discover_t;

/* Распаковка пакета с информацией об обнаруженном преобразователе интерфейсов */
bool_t discover_unpack(discover_t *discover, void *data, size_t size);

/* Формирование пакета с информацией об обнаурженном преобразователе интерфейсов */
ssize_t discover_pack(discover_t *discover, void *data, size_t size);

/* Вывод информации об обнаруженном преобразователе интерфейсов */
bool_t discover_print(discover_t *discover);

/* Структура данных, соответствующая команде установке настроек преобразователя интерфейсов */
typedef struct writeparams_s
{
  mac_t mac;
  unsigned char password[7];
  params_t params;
} writeparams_t;

/* Пароль по умолчанию в команде настройки преобразователя интерфейсов */
static const char DEFAULT_PASSWORD[] = {'1', '1', '0', '4', '1', '5'};

/* Разбор пакета для настройки преобразователя интерфейсов */
bool_t writeparams_unpack(writeparams_t *writeparams, void *data, size_t size);

/* Формирование пакета для настройки преобразователя интерфейсов */
ssize_t writeparams_pack(writeparams_t *writeparams, void *data, size_t size);

/* Вывод информации о настраиваемом преобразователе интерфейсов */
bool_t writeparams_print(writeparams_t *writeparams);

#endif

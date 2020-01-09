#include <stdio.h>
#include "proto.h"

#define PARAMS_SIZE 28

/* Распаковка настроек преобразователя интерфейсов из пакета */
bool_t params_unpack(params_t *params, void *data, size_t size)
{
  if (params == NULL)
  {
    fprintf(stderr, "params_unpack: params is NULL pointer.\n");
    return FALSE;
  }

  if (data == NULL)
  {
    fprintf(stderr, "params_unpack: data is NULL pointer.\n");
    return FALSE;
  }

  if (size != PARAMS_SIZE)
  {
    fprintf(stderr, "params_unpack: wrong data size.\n");
    return FALSE;
  }

  unsigned char *d = data;

  /* Целевой IP-адрес для режимов UCP_CLIENT и TCP_CLIENT */
  params->dest_ip.a = d[3];
  params->dest_ip.b = d[2];
  params->dest_ip.c = d[1];
  params->dest_ip.d = d[0];

  /* Целевой порт для режимов UCP_CLIENT и TCP_CLIENT */
  params->dest_port = d[4] | (d[5] << 8);

  /* IP-адрес */
  params->ip.a = d[9];
  params->ip.b = d[8];
  params->ip.c = d[7];
  params->ip.d = d[6];

  /* Порт */
  params->port = d[10] | (d[11] << 8);

  /* IP-адрес шлюза по умолчанию */
  params->gateway.a = d[15];
  params->gateway.b = d[14];
  params->gateway.c = d[13];
  params->gateway.d = d[12];

  /* Режим работы преобразователя интерфейсов */
  switch (d[16])
  {
    case 0:
      params->mode = UDP_CLIENT;
      break;
    case 1:
      params->mode = TCP_CLIENT;
      break;
    case 2:
      params->mode = UDP_SERVER;
      break;
    case 3:
      params->mode = TCP_SERVER;
      break;
    default:
      fprintf(stderr, "params_unpack: wrong mode %d\n", d[16]);
      return FALSE;
  }

  /* Битовая скорость последовательного интерфейса */
  params->bitrate = d[17] | (d[18] << 8) | (d[19] << 16);

  /* Количество бит данных последовательного интерфейса */
  params->data_bits = (d[20] & 0x03) + 5;

  /* Количество стоп-битов последовательного интерфейса */
  params->stop_bits = ((d[20] & 0x04) >> 2) + 1;

  /* Режим контроля чётности последовательного интерфейса */
  if (((d[20] & 0x08) >> 3) == 1)
  {
    switch ((d[20] & 0x30) >> 4)
    {
      case 0:
        params->parity = PARITY_ODD;
        break;
      case 1:
        params->parity = PARITY_EVEN;
        break;
      case 2:
        params->parity = PARITY_MARK;
        break;
      case 3:
        params->parity = PARITY_CLEAR;
        break;
    }
  }
  else
  {
    params->parity = PARITY_DISABLED;
  }

  /* Проверка, что в старших битах нет непредусмотренных данных */
  unsigned high = d[20] & 0xC0;
  if (high != 0)
  {
    fprintf(stderr, "params_unpack: wrong word format high bits 0x%02X\n", high);
    return FALSE;
  }

  /* Идентификатор для режима TCP_CLIENT */
  params->flags = d[23];
  params->id = (d[21] << 8) | d[22];

  /* Маска подсети */
  params->netmask.a = d[27];
  params->netmask.b = d[26];
  params->netmask.c = d[25];
  params->netmask.d = d[24];

  return TRUE;
}

/* Упаковка настроек преобразователя интерфейсов в пакет */
ssize_t params_pack(params_t *params, void *data, size_t size)
{
  if (params == NULL)
  {
    fprintf(stderr, "params_pack: params is NULL pointer.\n");
    return -1;
  }

  if (data == NULL)
  {
    fprintf(stderr, "params_pack: data is NULL pointer.\n");
    return -1;
  }

  if (size < PARAMS_SIZE)
  {
    fprintf(stderr, "params_pack: not enough room in data buffer.\n");
    return -1;
  }

  unsigned char *d = data;

  /* Целевой IP-адрес для режимов UCP_CLIENT и TCP_CLIENT */
  d[3] = params->dest_ip.a;
  d[2] = params->dest_ip.b;
  d[1] = params->dest_ip.c;
  d[0] = params->dest_ip.d;

  /* Целевой порт для режимов UCP_CLIENT и TCP_CLIENT */
  d[4] = params->dest_port & 0xFF;
  d[5] = params->dest_port >> 8;

  /* IP-адрес */
  d[9] = params->ip.a;
  d[8] = params->ip.b;
  d[7] = params->ip.c;
  d[6] = params->ip.d;

  /* Порт */
  d[10] = params->port & 0xFF;
  d[11] = params->port >> 8;

  /* IP-адрес шлюза по умолчанию */
  d[15] = params->gateway.a;
  d[14] = params->gateway.b;
  d[13] = params->gateway.c;
  d[12] = params->gateway.d;

  /* Режим работы преобразователя интерфейсов */
  switch (params->mode)
  {
    case UDP_CLIENT:
      d[16] = 0;
      break;
    case TCP_CLIENT:
      d[16] = 1;
      break;
    case UDP_SERVER:
      d[16] = 2;
      break;
    case TCP_SERVER:
      d[16] = 3;
      break;
    default:
      fprintf(stderr, "params_pack: wrong mode %d\n", params->mode);
      return -1;
  }

  /* Битовая скорость последовательного интерфейса */
  d[17] = params->bitrate & 0x0000FF;
  d[18] = (params->bitrate & 0x00FF00) >> 8;
  d[19] = (params->bitrate & 0xFF0000) >> 16;

  /* Количество бит данных последовательного интерфейса */
  if ((params->data_bits < 5) || (params->data_bits > 8))
  {
    fprintf(stderr, "params_pack: wrong data bits value %d\n", params->data_bits);
    return -1;
  }
  d[20] = params->data_bits - 5;

  /* Количество стоп-битов последовательного интерфейса */
  if ((params->stop_bits < 1) || (params->stop_bits > 2))
  {
    fprintf(stderr, "params_pack: wrong stop bits value %d\n", params->stop_bits);
    return -1;
  }
  d[20] |= (params->stop_bits - 1) << 2;

  /* Режим контроля чётности последовательного интерфейса */
  switch (params->parity)
  {
    case PARITY_DISABLED:
      break;
    case PARITY_ODD:
      d[20] |= 0x08;
      break;
    case PARITY_EVEN:
      d[20] |= 0x18;
      break;
    case PARITY_MARK:
      d[20] |= 0x28;
      break;
    case PARITY_CLEAR:
      d[20] |= 0x38;
      break;
    default:
      fprintf(stderr, "params_pack: wrong parity %d\n", params->parity);
      return -1;
  }

  /* Дополнительные настройки */
  d[23] = params->flags;

  /* Идентификатор для режима TCP_CLIENT */
  d[22] = params->id & 0xFF;
  d[21] = params->id >> 8;

  /* Маска подсети */
  d[27] = params->netmask.a;
  d[26] = params->netmask.b;
  d[25] = params->netmask.c;
  d[24] = params->netmask.d;

  return PARAMS_SIZE;
}
 
/* Вывод настроек преобразователя интерфейсов */
bool_t params_print(params_t *params)
{
  /* Режим работы преобразователя интерфейсов */
  switch (params->mode)
  {
    case UDP_CLIENT:
      fprintf(stdout, "Mode:    UDP-client\n"
                      "DstIP:   %d.%d.%d.%d\n"
                      "DstPort: %d\n",
                      params->dest_ip.a, params->dest_ip.b, params->dest_ip.c, params->dest_ip.d,
                      params->dest_port);
      break;
    case TCP_CLIENT:
      fprintf(stdout, "Mode:    TCP-client\n"
                      "DstIP:   %d.%d.%d.%d\n"
                      "DstPort: %d\n",
                      params->dest_ip.a, params->dest_ip.b, params->dest_ip.c, params->dest_ip.d,
                      params->dest_port);
      if (params->flags & (PARAMS_FLAGS_CONNECT | PARAMS_FLAGS_DATA))
      {
        fprintf(stdout, "ID:      %d\n",
                        params->id);
      }
      break;
    case UDP_SERVER:
      fprintf(stdout, "Mode:    UDP-server\n");
      break;
    case TCP_SERVER:
      fprintf(stdout, "Mode:    TCP-server\n");
      break;
    default:
      return FALSE;
  }

  fprintf(stdout, "Flags:\n");
  if (params->flags & PARAMS_FLAGS_CONNECT)
  {
    fprintf(stdout, "         Send ID when conndected\n");
  }
  if (params->flags & PARAMS_FLAGS_DATA)
  {
    fprintf(stdout, "         Send ID with data\n");
  }
  if (params->flags & PARAMS_FLAGS_RS485)
  {
    fprintf(stdout, "         Use RS485\n");
  }
  if (params->flags & PARAMS_FLAGS_RS422)
  {
    fprintf(stdout, "         Use RS422\n");
  }
  if (params->flags & PARAMS_FLAGS_RESET)
  {
    fprintf(stdout, "         Reset after 30 failed connections\n");
  }
  if (params->flags & PARAMS_FLAGS_LINK)
  {
    fprintf(stdout, "         Enable link led if connected\n");
  }
  if (params->flags & PARAMS_FLAGS_INDEX)
  {
    fprintf(stdout, "         Use device index\n");
  }
  if (params->flags & PARAMS_FLAGS_RFC2217)
  {
    fprintf(stdout, "         Enable switching bitrate RFC2217\n");
  }
  if (params->flags == 0)
  {
    fprintf(stdout, "         None\n");
  }

  /* Настройки Ethernet-интерфейса */
  fprintf(stdout, "IP:      %d.%d.%d.%d\n"
                  "Netmask: %d.%d.%d.%d\n"
                  "Gateway: %d.%d.%d.%d\n"
                  "Port:    %d\n",
                  params->ip.a, params->ip.b, params->ip.c, params->ip.d,
                  params->netmask.a, params->netmask.b, params->netmask.c, params->netmask.d,
                  params->gateway.a, params->gateway.b, params->gateway.c, params->gateway.d,
                  params->port);

  /* Настройки последовательного интерфейса */
  switch (params->parity)
  {
    case PARITY_DISABLED:
      fprintf(stdout, "Serial:  %u/%u-N-%u\n",
                      params->bitrate, params->data_bits, params->stop_bits);
      break;
    case PARITY_ODD:
      fprintf(stdout, "Serial:  %u/%u-O-%u\n",
                      params->bitrate, params->data_bits, params->stop_bits);
      break;
    case PARITY_EVEN:
      fprintf(stdout, "Serial:  %u/%u-E-%u\n",
                      params->bitrate, params->data_bits, params->stop_bits);
      break;
    case PARITY_MARK:
      fprintf(stdout, "Serial:  %u/%u-M-%u\n",
                      params->bitrate, params->data_bits, params->stop_bits);
      break;
    case PARITY_CLEAR:
      fprintf(stdout, "Serial:  %u/%u-C-%u\n",
                      params->bitrate, params->data_bits, params->stop_bits);
      break;
    default:
      return FALSE;
  }

  return TRUE;
}

/* Возвращает TRUE, если значения всех полей двух параметров совпадают */
bool_t params_equal(params_t *a, params_t *b)
{
  if (a == NULL)
  {
    fprintf(stderr, "params_equal: params a is NULL pointer\n");
    return FALSE;
  }
  if (b == NULL)
  {
    fprintf(stderr, "params_equal: params b is NULL pointer\n");
    return FALSE;
  }

  if (a->mode != b->mode)
  {
    return FALSE;
  }
  if ((a->mode == TCP_CLIENT) || (a->mode == UDP_CLIENT))
  {
    if ((a->dest_ip.a != b->dest_ip.a) || 
        (a->dest_ip.b != b->dest_ip.b) || 
        (a->dest_ip.c != b->dest_ip.c) || 
        (a->dest_ip.d != b->dest_ip.d))
    {
      return FALSE;
    }
    if (a->dest_port != b->dest_port)
    {
      return FALSE;
    }
  }

  if (a->flags != b->flags)
  {
    return FALSE;
  }
  if (a->flags & (PARAMS_FLAGS_CONNECT | PARAMS_FLAGS_DATA))
  {
    if (a->id != b->id)
    {
      return FALSE;
    }
  }

  if ((a->ip.a != b->ip.a) ||
      (a->ip.b != b->ip.b) ||
      (a->ip.c != b->ip.c) ||
      (a->ip.d != b->ip.d))
  {
    return FALSE;
  }

  if ((a->netmask.a != b->netmask.a) ||
      (a->netmask.b != b->netmask.b) ||
      (a->netmask.c != b->netmask.c) ||
      (a->netmask.d != b->netmask.d))
  {
    return FALSE;
  }

  if ((a->gateway.a != b->gateway.a) ||
      (a->gateway.b != b->gateway.b) ||
      (a->gateway.c != b->gateway.c) ||
      (a->gateway.d != b->gateway.d))
  {
    return FALSE;
  }

  if (a->port != b->port)
  {
    return FALSE;
  }

  if (a->bitrate != b->bitrate)
  {
    return FALSE;
  }

  if (a->data_bits != b->data_bits)
  {
    return FALSE;
  }

  if (a->stop_bits != b->stop_bits)
  {
    return FALSE;
  }

  if (a->parity != b->parity)
  {
    return FALSE;
  }

  return TRUE;
}

#define DISCOVER_SIZE 35

/* Распаковка пакета с информацией об обнаруженном преобразователе интерфейсов */
bool_t discover_unpack(discover_t *discover, void *data, size_t size)
{
  if (discover == NULL)
  {
    fprintf(stderr, "discover_unpack: discover is NULL pointer.\n");
    return FALSE;
  }

  if (data == NULL)
  {
    fprintf(stderr, "discover_unpack: data is NULL pointer.\n");
    return FALSE;
  }

  if (size != DISCOVER_SIZE)
  {
    fprintf(stderr, "discover_unpack: wrong data size.\n");
    return FALSE;
  }

  unsigned char *d = data;

  /* MAC-адрес ответившего преобразователя интерфейсов */
  discover->mac.a = d[0];
  discover->mac.b = d[1];
  discover->mac.c = d[2];
  discover->mac.d = d[3];
  discover->mac.e = d[4];
  discover->mac.f = d[5];

  /* Версия прошивки ответившего преобразователя интерфейсов */
  discover->major_version = d[6] >> 4;
  discover->minor_version = d[6] & 0x0F;

  /* Настройки преобразователя интерфейсов */
  if (params_unpack(&(discover->params), &(d[7]), PARAMS_SIZE) == FALSE)
  {
    fprintf(stderr, "discover_unpack: params_unpack failed.\n");
    return FALSE;
  }

  return TRUE;
}

/* Формирование пакета с информацией об обнаурженном преобразователе интерфейсов */
ssize_t discover_pack(discover_t *discover, void *data, size_t size)
{
  if (discover == NULL)
  {
    fprintf(stderr, "discover_pack: discover is NULL pointer.\n");
    return -1;
  }

  if (data == NULL)
  {
    fprintf(stderr, "discover_pack: data is NULL pointer.\n");
    return -1;
  }

  if (size < DISCOVER_SIZE)
  {
    fprintf(stderr, "discover_pack: not enough room in data buffer.\n");
    return FALSE;
  }

  unsigned char *d = data;

  /* MAC-адрес ответившего преобразователя интерфейсов */
  d[0] = discover->mac.a;
  d[1] = discover->mac.b;
  d[2] = discover->mac.c;
  d[3] = discover->mac.d;
  d[4] = discover->mac.e;
  d[5] = discover->mac.f;

  /* Версия прошивки ответившего преобразователя интерфейсов */
  if (discover->major_version > 0xF)
  {
    fprintf(stderr, "discover_pack: too big major version value %d.\n", discover->major_version);
    return -1;
  }
  if (discover->minor_version > 0xF)
  {
    fprintf(stderr, "discover_pack: too big minor version value %d.\n", discover->minor_version);
    return -1;
  }
  d[6] = (discover->major_version << 4) | discover->minor_version;

  /* Настройки преобразователя интерфейсов */
  if (params_pack(&(discover->params), &(d[7]), PARAMS_SIZE) == -1)
  {
    fprintf(stderr, "discover_pack: params_pack failed.\n");
    return -1;
  }

  return DISCOVER_SIZE;
}

/* Вывод информации об обнаруженном преобразователе интерфейсов */
bool_t discover_print(discover_t *discover)
{
  if (discover == NULL)
  {
    fprintf(stderr, "discover_print: discover is NULL pointer.\n");
    return FALSE;
  }

  /* Вывод MAC-адреса и версии прошивки преобразователя интерфейсов */
  fprintf(stdout, "MAC:     %02X:%02X:%02X:%02X:%02X:%02X\n"
                  "Version: %d.%d\n",
                  discover->mac.a, discover->mac.b, discover->mac.c, discover->mac.d, discover->mac.e, discover->mac.f,
                  discover->major_version, discover->minor_version);

  /* Вывод настроек преобразователя интерфейсов */
  if (params_print(&(discover->params)) == FALSE)
  {
    fprintf(stderr, "discover_print: params_print failed.\n");
    return FALSE;
  }

  return TRUE;
}

#define WRITEPARAMS_SIZE 40

/* Разбор пакета для настройки преобразователя интерфейсов */
bool_t writeparams_unpack(writeparams_t *writeparams, void *data, size_t size)
{
  if (writeparams == NULL)
  {
    fprintf(stderr, "writeparams_unpack: writeparams is NULL pointer.\n");
    return FALSE;
  }

  if (data == NULL)
  {
    fprintf(stderr, "writeparams_unpack: data is NULL pointer.\n");
    return FALSE;
  }

  if (size != WRITEPARAMS_SIZE)
  {
    fprintf(stderr, "writeparams_unpack: wrong data size.\n");
    return FALSE;
  }

  unsigned char *d = data;

  /* Извлекаем MAC-адрес настраиваемого преобразователя интерфейсов */
  writeparams->mac.a = d[0];
  writeparams->mac.b = d[1];
  writeparams->mac.c = d[2];
  writeparams->mac.d = d[3];
  writeparams->mac.e = d[4];
  writeparams->mac.f = d[5];

  /* Извлекаем пароль настраиваемого преобразователя интерфейсов */
  writeparams->password[0] = d[6];
  writeparams->password[1] = d[7];
  writeparams->password[2] = d[8];
  writeparams->password[3] = d[9];
  writeparams->password[4] = d[10];
  writeparams->password[5] = d[11];
  writeparams->password[6] = '\0';

  /* Извлекаем настройки настраиваемого преобразователя интерфейсов */
  if (params_unpack(&(writeparams->params), &(d[12]), PARAMS_SIZE) == FALSE)
  {
    fprintf(stderr, "writeparams_unpack: params_unpack failed.\n");
    return FALSE;
  }

  return TRUE;
}

/* Формирование пакета для настройки преобразователя интерфейсов */
ssize_t writeparams_pack(writeparams_t *writeparams, void *data, size_t size)
{
  if (writeparams == NULL)
  {
    fprintf(stderr, "writeparams_pack: writeparams is NULL pointer.\n");
    return -1;
  }

  if (data == NULL)
  {
    fprintf(stderr, "writeparams_pack: data is NULL pointer.\n");
    return -1;
  }

  if (size < WRITEPARAMS_SIZE)
  {
    fprintf(stderr, "writeparams_pack: not enough room in data buffer.\n");
    return -1;
  }

  unsigned char *d = data;

  /* Записываем MAC-адрес настраиваемого преобразователя интерфейсов */
  d[0] = writeparams->mac.a;
  d[1] = writeparams->mac.b;
  d[2] = writeparams->mac.c;
  d[3] = writeparams->mac.d;
  d[4] = writeparams->mac.e;
  d[5] = writeparams->mac.f;

  /* Записываем пароль настраиваемого преобразователя интерфейсов */
  d[6] = writeparams->password[0];
  d[7] = writeparams->password[1];
  d[8] = writeparams->password[2];
  d[9] = writeparams->password[3];
  d[10] = writeparams->password[4];
  d[11] = writeparams->password[5];

  /* Записываем настройки преобразователя интерфейсов */
  if (params_pack(&(writeparams->params), &(d[12]), PARAMS_SIZE) == -1)
  {
    fprintf(stderr, "writeparams_pack: params_pack failed.\n");
    return -1;
  }

  return WRITEPARAMS_SIZE;
}

/* Вывод информации о настраиваемом преобразователе интерфейсов */
bool_t writeparams_print(writeparams_t *writeparams)
{
  if (writeparams == NULL)
  {
    fprintf(stderr, "writeparams_print: writeparams is NULL pointer.\n");
    return FALSE;
  }

  /* Выводим MAC-адрес настраиваемого преобразователя интерфейсов и пароль */
  fprintf(stdout, "MAC:     %02X:%02X:%02X:%02X:%02X:%02X\n"
                  "Passwd:  %s\n",
                  writeparams->mac.a, writeparams->mac.b, writeparams->mac.c, writeparams->mac.d, writeparams->mac.e, writeparams->mac.f, 
                  writeparams->password);

  /* Выводим настройки настраиваемого преобразователя интерфейсов */
  if (params_print(&(writeparams->params)) == FALSE)
  {
    fprintf(stderr, "writeparams_print: params_print failed.\n");
    return FALSE;
  }

  return TRUE;
}

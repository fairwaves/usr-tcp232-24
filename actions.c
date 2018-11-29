#include <stdio.h>
#include <string.h>
#include "types.h"
#include "proto.h"
#include "net.h"
#include "actions.h"

/* Вывод справки об использовании программы */
int help(char *executed_pathname)
{
  fprintf(stderr, "Usage: %s discover <interface>\n"
                  "       %s config|change <interface> <mac> <options>\n"
                  "Options:\n"
                  "  Serial interface:\n"
                  "    -1               - use one stop bit\n"
                  "    -2               - use two stop bits\n"
                  "    -5               - use five data bits\n"
                  "    -6               - use six data bits\n"
                  "    -7               - use seven data bits\n"
                  "    -8               - use eight data bits\n"
                  "    -N               - don't use parity bit\n"
                  "    -E               - use even parity bit control\n"
                  "    -O               - use odd parity bit control\n"
                  "    -M               - always set parity bit\n"
                  "    -C, -S           - always clear parity bit\n"
                  "    -r <bitrate>     - use specified bitrate, bits/s\n"
                  "    -w <word-format> - use specified data bits, parity control mode\n"
                  "                       and stop bits\n"
                  "                       Example: 8N1 - 8 data bits, no parity control, 1 stop bit\n"
                  "    -s <serial>      - use specified bitrate, data bits, parity control mode\n"
                  "                       and stop bits. Examples: 9600/8N1, 19200/7-E-1\n"
                  "  Ethernet interface:\n"
                  "    -i <ip>          - use specified IP\n"
                  "    -n <netmask>     - use specified subnetwork mask\n"
                  "    -g <gateway>     - use specified gateway\n"
                  "  Converter mode:\n"
                  "    -u               - use UDP-client mode\n"
                  "    -U               - use UDP-server mode\n"
                  "    -t               - use TCP-client mode\n"
                  "    -T               - use TCP-server mode\n"
                  "    -m <port>        - use specified source port\n"
                  "                       (for UDP-client and TCP-client modes)\n"
                  "                       or listen specified port\n"
                  "                       (for UDP-server and TCP-server modes)\n"
                  "    -d <dest-ip>     - use specified destination ip\n"
                  "                       (for UDP-client and TCP-client modes)\n"
                  "    -p <dest-port>   - use specified destination port\n"
                  "                       (for UDP-client and TCP-client modes)\n"
                  "  Additional settings:\n"
                  "    -I <id>          - use specified converter identifier\n"
                  "                       (only in TCP-client mode and enabled flags\n"
                  "                       connect-id or/and data-id)\n"
                  "    -f <flags>       - use only specified comma-separated flags\n"
                  "    -f +<flags>      - enable specified comma-separated flags\n"
                  "    -f -<flags>      - disable specified comma-separated flags\n"
                  "  Flags:\n"
                  "    connect-id       - send converter identifier after connection was\n"
                  "                       established\n"
                  "    data-id          - send data with converter identifier\n"
                  "    rs485            - use interface RS-485 instead of RS-232\n"
                  "    rs422            - use interface RS-422 instead of RS-232\n"
                  "    reset            - reset converter after 30 failed connections to server\n"
                  "                       (only in TCP-client mode)\n"
                  "    link             - enable link led when connection is established\n"
                  "    index            - enable indexing of incoming connections\n"
                  "                       (only for TCP-server mode)\n"
                  "    rfc2217          - enable RFC2217 functions for temporarily changing of\n"
                  "                       serial settings (only before next restart of converter)\n",
                  executed_pathname, executed_pathname);
  return 1;
}

/* Обнаружение преобразователей интерфейсов */
int discover(char *interface)
{
  if (interface == NULL)
  {
    fprintf(stderr, "discover: interface is NULL\n");
    return 1;
  }

  /* Создаём сокет */
  int sock = create_socket(interface);
  if (sock == -1)
  {
    fprintf(stderr, "discover: failed to create socket\n");
    return 1;
  }

  /* Запускаем обнаружение преобразователей интерфейсов */
  if (discover_start(sock) == FALSE)
  {
    fprintf(stderr, "discover: discover_start failed\n");
    return 1;
  }

  /* Для каждого из обнаруженных преобразователей интерфейсов выводим обнаруженные данные на экран */
  discover_t discovered;
  while (discover_next(sock, &discovered) == TRUE)
  {
    if (discover_print(&discovered) == FALSE)
    {
      fprintf(stderr, "discover: warning, failed to print discovered data\n");
    }
  }
  /* Прекращаем обнаружение */
  discover_stop();

  /* Закрываем сокет */
  if (close(sock) == -1)
  {
    fprintf(stderr, "discover: warning, failed to close socket\n");
  }

  return 0;
}

typedef struct flag_mask_s
{
  char *flag;
  unsigned char mask;
} flag_mask_t;

flag_mask_t flags_masks[] = {
  {"connect-id", PARAMS_FLAGS_CONNECT},
  {"data-id",    PARAMS_FLAGS_DATA},
  {"rs485",      PARAMS_FLAGS_RS485},
  {"rs422",      PARAMS_FLAGS_RS422},
  {"reset",      PARAMS_FLAGS_RESET},
  {"link",       PARAMS_FLAGS_LINK},
  {"index",      PARAMS_FLAGS_INDEX},
  {"rfc2217",    PARAMS_FLAGS_RFC2217}
};

size_t flags_masks_num = sizeof(flags_masks) / sizeof(flag_mask_t);

/* Функция возвращает количество символов, которое содержится в префиксе prefix,
   если префикс является началом строки str. Если же префикс в начале строки не
   найден, возвращается 0 */
size_t strstart(char *str, char *prefix)
{
  if (str == NULL)
  {
    fprintf(stderr, "strstart: str is NULL pointer\n");
    return 0;
  }

  if (prefix == NULL)
  {
    fprintf(stderr, "strstart: prefix is NULL pointer\n");
    return 0;
  }

  size_t n = 0;
  while (*prefix != '\0')
  {
    /* Префикс ещё не кончился, а строка уже закончилась */
    if (*str == '\0')
    {
      return 0;
    }

    /* Начало строки не совпадает с префиксом */
    if (*str != *prefix)
    {
      return 0;
    }

    /* Совпадение очередного символа найдено */
    n++;
    str++;
    prefix++;
  }

  /* Префикс закончился, возвращаем количество совпавших символов */
  return n;
}

/* Применение дополнительных флагов, указанных в опциях программы */
bool_t apply_flags(unsigned char *flags, char *sflags)
{
  if (flags == NULL)
  {
    fprintf(stderr, "apply_flags: flags is NULL pointer\n");
    return FALSE;
  }

  if (sflags == NULL)
  {
    fprintf(stderr, "apply_flags: sflags is NULL pointer\n");
    return FALSE;
  }

  /* Операция с битовой маской флагов по умолчанию - присвоить */
  char operation = '=';
  /* Если указана другая операция, то запоминаем её */
  if ((*sflags == '-') || (*sflags == '+'))
  {
    operation = *sflags;
    sflags++;
  }

  /* Изначально маска не содержит флагов */
  unsigned char mask = 0;

  /* Пока в строке со списком флагов есть символы,
     пытаемся продолжать поиск флагов в ней */
  while (*sflags != '\0')
  {
    /* Если ранее уже был найден хоть один флаг,
       то перед следующим флагом должен быть разделитель */
    if (mask > 0)
    {
      if (*sflags != ',')
      {
        fprintf(stderr, "apply_flags: syntax error, unparsed rest of string %s\n", sflags);
        return FALSE;
      }
      sflags++;
    }

    /* Очередной флаг в строке ещё не найден */
    bool_t parsed = FALSE;

    /* Сравниваем очередной флаг со списком всех возможных флагов */
    for(size_t i = 0; i < flags_masks_num; i++)
    {
      size_t n = strstart(sflags, flags_masks[i].flag);
      /* При обнаружении совпадения, добавляем флаг к маске */
      if (n > 0)
      {
        mask |= flags_masks[i].mask;
        sflags += n;
        parsed = TRUE;
        break;
      }
    }

    /* Если флаг в строке не найден, то это не известный флаг */
    if (parsed == FALSE)
    {
      fprintf(stderr, "apply_flags: wrong flag, unparsed rest of string %s\n", sflags);
      return FALSE;
    }
  }

  /* В зависимости от операции, изменяем значение флагов */
  if (operation == '-')
  {
    *flags &= ~mask;
  }
  else if (operation == '+')
  {
    *flags |= mask;
  }
  else
  {
    *flags = mask;
  }
  
  return TRUE;
}

/* Изменение настроек преобразователя интерфейса в соответствии со
   значениями, указанными в опциях */
bool_t apply_options(params_t *params, int nopts, char *vopts[])
{
  for(int i = 0; i < nopts; i++)
  {
    if (strcmp(vopts[i], "-1") == 0)
    {
      params->stop_bits = 1;
    }
    else if (strcmp(vopts[i], "-2") == 0)
    {
      params->stop_bits = 2;
    }
    else if (strcmp(vopts[i], "-5") == 0)
    {
      params->data_bits = 5;
    }
    else if (strcmp(vopts[i], "-6") == 0)
    {
      params->data_bits = 6;
    }
    else if (strcmp(vopts[i], "-7") == 0)
    {
      params->data_bits = 7;
    }
    else if (strcmp(vopts[i], "-8") == 0)
    {
      params->data_bits = 8;
    }
    else if (strcmp(vopts[i], "-N") == 0)
    {
      params->parity = PARITY_DISABLED;
    }
    else if (strcmp(vopts[i], "-E") == 0)
    {
      params->parity = PARITY_EVEN;
    }
    else if (strcmp(vopts[i], "-O") == 0)
    {
      params->parity = PARITY_ODD;
    }
    else if (strcmp(vopts[i], "-M") == 0)
    {
      params->parity = PARITY_MARK;
    }
    else if (strcmp(vopts[i], "-C") == 0)
    {
      params->parity = PARITY_CLEAR;
    }
    else if (strcmp(vopts[i], "-r") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -r\n");
        return FALSE;
      }

      if (parse_bitrate(&(params->bitrate), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -r\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-w") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -w\n");
        return FALSE;
      }

      if (parse_word_format(&(params->data_bits), &(params->parity), &(params->stop_bits), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -w\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-s") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -s\n");
        return FALSE;
      }

      if (parse_serial(&(params->bitrate), &(params->data_bits), &(params->parity), &(params->stop_bits), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -s\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-i") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -i\n");
        return FALSE;
      }

      if (parse_ip(&(params->ip), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -i\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-n") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -n\n");
        return FALSE;
      }

      if (parse_ip(&(params->netmask), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -n\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-g") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -g\n");
        return FALSE;
      }

      if (parse_ip(&(params->gateway), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -g\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-u") == 0)
    {
      params->mode = UDP_CLIENT;
    }
    else if (strcmp(vopts[i], "-U") == 0)
    {
      params->mode = UDP_SERVER;
    }
    else if (strcmp(vopts[i], "-t") == 0)
    {
      params->mode = TCP_CLIENT;
    }
    else if (strcmp(vopts[i], "-T") == 0)
    {
      params->mode = TCP_SERVER;
    }
    else if (strcmp(vopts[i], "-d") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -d\n");
        return FALSE;
      }

      if (parse_ip(&(params->dest_ip), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -d\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-m") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -m\n");
        return FALSE;
      }

      if (parse_port(&(params->port), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -m\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-p") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -p\n");
        return FALSE;
      }

      if (parse_port(&(params->dest_port), vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -p\n");
        return FALSE;
      }
    }
    else if (strcmp(vopts[i], "-I") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -I\n");
        return FALSE;
      }

      unsigned id = 0;
      if (parse_unsigned(&id, vopts[i], 0xFFFF) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -I\n");
        return FALSE;
      }

      params->id = (unsigned short)id;
    }
    else if (strcmp(vopts[i], "-f") == 0)
    {
      i++;
      if (i >= nopts)
      {
        fprintf(stderr, "apply_options: missing argument of option -f\n");
        return FALSE;
      }

      unsigned char flags = params->flags;
      if (apply_flags(&flags, vopts[i]) == FALSE)
      {
        fprintf(stderr, "apply_options: wrong argument value of option -f\n");
        return FALSE;
      }

      params->flags = flags;
    }
  }
  return TRUE;
}

/* Замена текущих настроек преобразователя интерфейсов */
int config(char *interface, char *dest_mac, int nopts, char *vopts[])
{
  if (interface == NULL)
  {
    fprintf(stderr, "config: interface is NULL\n");
    return -1;
  }

  if (dest_mac == NULL)
  {
    fprintf(stderr, "config: dest_mac is NULL\n");
    return 1;
  }

  if (nopts < 0)
  {
    fprintf(stderr, "config: negative number of options\n");
    return 1;
  }

  /* Разбираем MAC-адрес настраиваемого преобразователя интерфейсов */
  mac_t mac;
  if (parse_mac(&mac, dest_mac) == FALSE)
  {
    fprintf(stderr, "config: failed to parse MAC-address\n");
    return 1;
  }

  /* Создаём сокет */
  int sock = create_socket(interface);
  if (sock == -1)
  {
    fprintf(stderr, "config: failed to create socket\n");
    return 1;
  }

  /* Готовим настройки по умолчанию */
  params_t params = DEFAULT_PARAMS;

  /* Применяем к настройкам по умолчанию указанные опции */
  if (apply_options(&params, nopts, vopts) == FALSE)
  {
    fprintf(stderr, "config: apply_options is failed\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "config: warning, failed to close socket\n");
    }
    return 1;
  }

  /* Применяем новые настройки к преобразователю интерфейсов */
  if (apply_params(sock, &mac, &params) == FALSE)
  {
    fprintf(stderr, "config: apply_params is failed\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "config: warning, failed to close socket\n");
    }
    return 1;
  }

  /* Закрываем сокет */
  if (close(sock) == -1)
  {
    fprintf(stderr, "config: warning, failed to close socket\n");
  }
  return 0;
}

/* Изменение текущих настроек преобразователя интерфейсов */
int change(char *interface, char *dest_mac, int nopts, char *vopts[])
{
  if (interface == NULL)
  {
    fprintf(stderr, "change: interface is NULL\n");
    return -1;
  }

  if (dest_mac == NULL)
  {
    fprintf(stderr, "change: dest_mac is NULL\n");
    return 1;
  }

  if (nopts < 0)
  {
    fprintf(stderr, "change: negative number of options\n");
    return 1;
  }

  /* Разбираем MAC-адрес настраиваемого преобразователя интерфейсов */
  mac_t mac;
  if (parse_mac(&mac, dest_mac) == FALSE)
  {
    fprintf(stderr, "change: failed to parse MAC-address\n");
    return 1;
  }

  /* Создаём сокет */
  int sock = create_socket(interface);
  if (sock == -1)
  {
    fprintf(stderr, "change: failed to create socket\n");
    return 1;
  }

  discover_t discovered;
  /* Обнаруживаем текущие настройки преобразователя интерфейсов */
  if (search(sock, &mac, &discovered) == FALSE)
  {
    fprintf(stderr, "change: failed to search configured converter\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "change: warning, failed to close socket\n");
    }
    return 1;
  }

  /* Применяем к найденным настройкам указанные опции */
  if (apply_options(&(discovered.params), nopts, vopts) == FALSE)
  {
    fprintf(stderr, "change: apply_options is failed\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "change: warning, failed to close socket\n");
    }
    return 1;
  }

  /* Применяем новые настройки к преобразователю интерфейсов */
  if (apply_params(sock, &mac, &(discovered.params)) == FALSE)
  {
    fprintf(stderr, "change: apply_params is failed\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "change: warning, failed to close socket\n");
    }
    return 1;
  }

  /* Закрываем сокет */
  if (close(sock) == -1)
  {
    fprintf(stderr, "change: warning, failed to close socket\n");
  }
  return 0;
}

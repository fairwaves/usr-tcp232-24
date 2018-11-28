#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "types.h"

/* Разбор строки с неотрицательным целым числом, значение которого не превышает max */
char *_parse_unsigned(unsigned *result, char *s, unsigned max)
{
  if (s == NULL)
  {
    fprintf(stderr, "_parse_unsigned: s is NULL pointer.\n");
    return NULL;
  }

  /* Выполняем преобразование строки в число */
  errno = 0;
  unsigned long n = strtoul(s, &s, 0);

  /* Анализируем ошибки переполнения */
  if ((errno == ERANGE) || (n > max))
  {
    return NULL;
  }

  /* Записываем результат разбора строки, если он нужен вызывающей строне */
  if (result != NULL)
  {
    *result = (unsigned)n;
  }

  /* Возвращаем указатель на неразобранный остаток строки */
  return s;
}

/* Разбор строки с неотрицательным целым числом, значение которого не превышает max.
   Пробелы в начале и в конце строки пропускаются */
bool_t parse_unsigned(unsigned *result, char *s, unsigned max)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_unsigned: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Разбираем строку с числом */
  s = _parse_unsigned(result, s, max);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  return TRUE;
}

bitrate_t bitrates[] = {
  300, 600, 1200, 2400, 4800, 9600, 19200,
  38400, 57600, 115200, 230400, 460800, 921600
};

/* Разбор строки с битовой скоростью */
char *_parse_bitrate(bitrate_t *bitrate, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "_parse_bitrate: s is NULL pointer.\n");
    return NULL;
  }

  /* Ищем в строке число */
  unsigned br;
  s = _parse_unsigned(&br, s, 0xFFFFFF);

  /* Если найти не удалось или возникло переполнение */
  if (s == NULL)
  {
    return NULL;
  }

  /* Проверяем, указана ли стандартная битовая скорость */
  bool_t found = FALSE;
  for(unsigned i=0; i < sizeof(bitrates) / sizeof(bitrates[0]); i++)
  {
    if (bitrates[i] == br)
    {
      found = TRUE;
    }
  }

  /* Если битовая скорость - не стандартная */
  if (!found)
  {
    return NULL;
  }

  /* Записываем результат, если он нужен */
  if (bitrate != NULL)
  {
    *bitrate = (bitrate_t)br;
  }

  return s; 
}

/* Разбор строки с битовой скоростью.
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_bitrate(bitrate_t *bitrate, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_bitrate: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Ищем в строке число */
  s = _parse_bitrate(bitrate, s);

  /* Если найти не удалось или возникло переполнение */
  if (s == NULL)
  {
    return FALSE;
  }

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  return TRUE;
}

/* Разбор символа с количеством битов данных */
int data_bits(char c)
{
  switch (c)
  {
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
  }
  return -1;
}

/* Разбор символа контроля чётности */
parity_t parity(char c)
{
  switch (c)
  {
    case 'N':
    case 'n':
      return PARITY_DISABLED;
    case 'O':
    case 'o':
      return PARITY_ODD;
    case 'E':
    case 'e':
      return PARITY_EVEN;
    case 'M':
    case 'm':
      return PARITY_MARK;
    case 'S':
    case 's':
    case 'C':
    case 'c':
      return PARITY_CLEAR;
  }
  return PARITY_WRONG;
}

/* Разбор символа с количеством стоп-битов */
int stop_bits(char c)
{
  switch (c)
  {
    case '1':
      return 1;
    case '2':
      return 2;
  }
  return -1;
}

/* Разбор строки с форматом слова в виде 8N1 или 8-N-1 */
char *_parse_word_format(unsigned char *db, parity_t *p, unsigned char *sb, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "_parse_word_format: s is NULL pointer.\n");
    return NULL;
  }

  /* Ищем символ с количеством бит данных */
  int _db = data_bits(*s);
  if (_db == -1)
  {
    return NULL;
  }
  s++;

  /* Если есть разделитель, то пропускаем его и запоминаем, что он был */
  bool_t delimiter = FALSE;
  if (*s == '-')
  {
    delimiter = TRUE;
    s++;
  }

  /* Ищем символ контроля чётности */
  parity_t _p = parity(*s);
  if (_p == PARITY_WRONG)
  {
    return NULL;
  }
  s++;

  /* Если должен быть разделитель, то проверяем его наличие */
  if (delimiter == TRUE)
  {
    if (*s != '-')
    {
      return NULL;
    }
    s++;
  }

  /* Ищем символ с количеством стоп-битов */
  int _sb = stop_bits(*s);
  if (_sb == -1)
  {
    return NULL;
  }
  s++;


  /* Записываем результаты, если они нужны */
  if (db != NULL)
  {
    *db = (unsigned char)_db;
  }
  if (p != NULL)
  {
    *p = _p;
  }
  if (sb != NULL)
  {
    *sb = (unsigned char)_sb;
  }

  return s; 
}

/* Разбор строки с форматом слова в виде 8N1 или 8-N-1.
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_word_format(unsigned char *db, parity_t *p, unsigned char *sb, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_word_format: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Ищем в строке формат слова */
  s = _parse_word_format(db, p, sb, s);

  /* Если найти не удалось */
  if (s == NULL)
  {
    return FALSE;
  }

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  return TRUE;
}

/* Разбор строки с настройками последовательного порта в виде 9600/8N1 или 9600/8-N-1.
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_serial(bitrate_t *br, unsigned char *db, parity_t *p, unsigned char *sb, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_serial: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Ищем в строке битовую скорость */
  s = _parse_bitrate(br, s);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Ищем разделитель */
  if (*s != '/')
  {
    return FALSE;
  }
  s++;

  /* Ищем в строке формат слова */
  s = _parse_word_format(db, p, sb, s);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  return TRUE;
}

/* Разбор строки с номером порта */
bool_t parse_port(port_t *port, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_port: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Ищем в строке число */
  unsigned p;
  s = _parse_unsigned(&p, s, 0xFFFF);

  /* Если найти не удалось или возникло переполнение */
  if (s == NULL)
  {
    return FALSE;
  }

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  /* Записываем результат, если он нужен */
  if (port != NULL)
  {
    *port = (port_t)p;
  }

  return TRUE;
}

/* Разбор строки с IP-адресом */
bool_t parse_ip(ip_t *ip, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_ip: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Разбираем первый октет IP-адреса */
  unsigned a;
  s = _parse_unsigned(&a, s, 255);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Ищем точку после первого октета */
  if (*s != '.')
  {
    return FALSE;
  }
  s++;

  /* Разбираем второй октет IP-адреса */
  unsigned b;
  s = _parse_unsigned(&b, s, 255);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Ищем точку после второго октета */
  if (*s != '.')
  {
    return FALSE;
  }
  s++;

  /* Разбираем третий октет IP-адреса */
  unsigned c;
  s = _parse_unsigned(&c, s, 255);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Ищем точку после третьего октета */
  if (*s != '.')
  {
    return FALSE;
  }
  s++;

  /* Разбираем четвёртый октет IP-адреса */
  unsigned d;
  s = _parse_unsigned(&d, s, 255);
  if (s == NULL)
  {
    return FALSE;
  }

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  /* Записываем результат, если он нужен */
  if (ip != NULL)
  {
    ip->a = (unsigned char)a;
    ip->b = (unsigned char)b;
    ip->c = (unsigned char)c;
    ip->d = (unsigned char)d;
  }

  return TRUE;
}

/* Преобразует шестнадцатеричный символ с цифрой в цифру.
   Возвращает -1, если символ не содержит шестнадцатеричной цифры */
int hex_digit(char c)
{
  switch (c)
  {
    case '0':
      return 0;
    case '1':
      return 1;    
    case '2':
      return 2;    
    case '3':
      return 3;    
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'a':
    case 'A':
      return 10;
    case 'b':
    case 'B':
      return 11;
    case 'c':
    case 'C':
      return 12;
    case 'd':
    case 'D':
      return 13;
    case 'e':
    case 'E':
      return 14;
    case 'f':
    case 'F':
      return 15;
  }
  return -1;
}

/* Разбор одного октета MAC-адреса, состоящего из двух символов с шестнадцатеричными цифрами.
   Вспомогательная функция для функций parse_mac1, parse_mac2, parse_mac3 */
char *parse_octet(unsigned char *n, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_octet: s is NULL pointer.\n");
    return NULL;
  }

  /* Разбираем первую цифру */
  int a = hex_digit(*s);
  if (a == -1)
  {
    return NULL;
  }
  s++;

  /* Разбираем вторую цифру */
  int b = hex_digit(*s);
  if (b == -1)
  {
    return NULL;
  }
  s++;

  /* Записываем результат, если он нужен */
  if (n != NULL)
  {
    *n = (a << 4) | b;
  }

  return s;
}

/* Разбор MAC-адреса в формате 00:11:22:33:44:55 или 00-11-22-33-44-55
   Вспомогательная функция для функции parse_mac */
char *parse_mac1(mac_t *mac, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_mac1: s is NULL pointer.\n");
    return NULL;
  }

  /* Разбираем первый октет */
  unsigned char a;
  s = parse_octet(&a, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Определяем символ-разделитель - ':' или '-' */
  int delimiter = *s;
  if ((delimiter != ':') && (delimiter != '-'))
  {
    return NULL;
  }
  s++;

  /* Разбираем второй октет и проверяем наличие разделителя */
  unsigned char b;
  s = parse_octet(&b, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != delimiter)
  {
    return NULL;
  }
  s++;
  
  /* Разбираем третий октет и проверяем наличие разделителя */
  unsigned char c;
  s = parse_octet(&c, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != delimiter)
  {
    return NULL;
  }
  s++;

  /* Разбираем четвёртый октет и проверяем наличие разделителя */
  unsigned char d;
  s = parse_octet(&d, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != delimiter)
  {
    return NULL;
  }
  s++;

  /* Разбираем пятый октет и проверяем наличие разделителя */
  unsigned char e;
  s = parse_octet(&e, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != delimiter)
  {
    return NULL;
  }
  s++;

  /* Разбираем шестой октет */
  unsigned char f;
  s = parse_octet(&f, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Записываем результат, если он нужен */
  if (mac != NULL)
  {
    mac->a = a;
    mac->b = b;
    mac->c = c;
    mac->d = d;
    mac->e = e;
    mac->f = f;
  }
  return s;
}

/* Разбор MAC-адреса в форматах 0011:2233:4455, 0011-2233-4455 или 0011.2233.4455
   Вспомогательная функция для функции parse_mac */
char *parse_mac2(mac_t *mac, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_mac2: s is NULL pointer.\n");
    return NULL;
  }

  /* Разбираем первый и второй октеты */
  unsigned char a;
  s = parse_octet(&a, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char b;
  s = parse_octet(&b, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Определяем символ-разделитель - ':', '-' или '.' */
  int delimiter = *s;
  if ((delimiter != ':') && (delimiter != '-') && (delimiter != '.'))
  {
    return NULL;
  }
  s++;

  /* Разбираем третий и четвёртый октеты и проверяем наличие разделителя */
  unsigned char c;
  s = parse_octet(&c, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char d;
  s = parse_octet(&d, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != delimiter)
  {
    return NULL;
  }
  s++;

  /* Разбираем пятый и шестой октеты */
  unsigned char e;
  s = parse_octet(&e, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char f;
  s = parse_octet(&f, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Записываем результат, если он нужен */
  if (mac != NULL)
  {
    mac->a = a;
    mac->b = b;
    mac->c = c;
    mac->d = d;
    mac->e = e;
    mac->f = f;
  }
  return s;
}

/* Разбор MAC-адреса в форматах 001122:334455 или 001122-334455
   Вспомогательная функция для функции parse_mac */
char *parse_mac3(mac_t *mac, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_mac3: s is NULL pointer.\n");
    return NULL;
  }

  /* Разбираем первый, второй и третий октеты */
  unsigned char a;
  s = parse_octet(&a, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char b;
  s = parse_octet(&b, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char c;
  s = parse_octet(&c, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Проверяем наличие символа-разделитель - ':' или '-' */
  if ((*s != ':') && (*s != '-'))
  {
    return NULL;
  }
  s++;

  /* Разбираем четвёртый, пятый и шестой октеты */
  unsigned char d;
  s = parse_octet(&d, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char e;
  s = parse_octet(&e, s);
  if (s == NULL)
  {
    return NULL;
  }
  unsigned char f;
  s = parse_octet(&f, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Записываем результат, если он нужен */
  if (mac != NULL)
  {
    mac->a = a;
    mac->b = b;
    mac->c = c;
    mac->d = d;
    mac->e = e;
    mac->f = f;
  }
  return s;
}

/* Разбор MAC-адреса в формате 001122334455
   Вспомогательная функция для функции parse_mac */
char *parse_mac4(mac_t *mac, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_mac4: s is NULL pointer.\n");
    return NULL;
  }

  /* Разбираем первый октет */
  unsigned char a;
  s = parse_octet(&a, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Разбираем второй октет */
  unsigned char b;
  s = parse_octet(&b, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Разбираем третий октет */
  unsigned char c;
  s = parse_octet(&c, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Разбираем четвёртый октет */
  unsigned char d;
  s = parse_octet(&d, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Разбираем пятый октет */
  unsigned char e;
  s = parse_octet(&e, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Разбираем шестой октет */
  unsigned char f;
  s = parse_octet(&f, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Записываем результат, если он нужен */
  if (mac != NULL)
  {
    mac->a = a;
    mac->b = b;
    mac->c = c;
    mac->d = d;
    mac->e = e;
    mac->f = f;
  }
  return s;
}

/* Разбор одного октета MAC-адреса, который может состоять из одного или двух символов,
   содержащих шестнадцатеричные цифры.
   Вспомогательная функция для функции parse_mac4 */
char *parse_octet2(unsigned char *octet, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_octet2: s is NULL pointer.\n");
    return NULL;
  }

  /* Если не удалось разобрать даже один символ, то октета нет */
  int a = hex_digit(*s);
  if (a == -1)
  {
    return NULL;
  }
  s++;

  /* Если не удалось разобрать второй символ, то октет состоит из одного символа */
  int b = hex_digit(*s);
  if (b == -1)
  {
    b = a;
    a = 0;
  }
  else
  {
    s++;
  }

  /* Записываем результат, если он нужен */
  if (octet != NULL)
  {
    *octet = (a << 4) | b;
  }
  return s;
}

/* Разбор MAC-адреса в формате 0:1:2:33:44:55
   Вспомогательная функция для функции parse_mac */
char *parse_mac5(mac_t *mac, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_mac5: s is NULL pointer.\n");
    return NULL;
  }

  /* Разбираем первый октет и проверяем наличие разделителя - ':' */
  unsigned char a;
  s = parse_octet2(&a, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != ':')
  {
    return NULL;
  }
  s++;
  
  /* Разбираем второй октет и проверяем наличие разделителя - ':' */
  unsigned char b;
  s = parse_octet2(&b, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != ':')
  {
    return NULL;
  }
  s++;

  /* Разбираем третий октет и проверяем наличие разделителя - ':' */
  unsigned char c;
  s = parse_octet2(&c, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != ':')
  {
    return NULL;
  }
  s++;

  /* Разбираем четвёртый октет и проверяем наличие разделителя - ':' */
  unsigned char d;
  s = parse_octet2(&d, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != ':')
  {
    return NULL;
  }
  s++;

  /* Разбираем пятый октет и проверяем наличие разделителя - ':' */
  unsigned char e;
  s = parse_octet2(&e, s);
  if (s == NULL)
  {
    return NULL;
  }
  if (*s != ':')
  {
    return NULL;
  }
  s++;

  /* Разбираем шестой октет */
  unsigned char f;
  s = parse_octet2(&f, s);
  if (s == NULL)
  {
    return NULL;
  }

  /* Записываем результат, если он нужен */
  if (mac != NULL)
  {
    mac->a = (unsigned char)a;
    mac->b = (unsigned char)b;
    mac->c = (unsigned char)c;
    mac->d = (unsigned char)d;
    mac->e = (unsigned char)e;
    mac->f = (unsigned char)f;
  }
  return s;
}

/* Разбор строки с MAC-адресом. Принимается один из форматов:
   00:11:22:33:44:55
   00-11-22-33-44-55
   0011:2233:4455
   0011-2233-4455
   001122:334455
   001122-334455
   00112233445566
   0:1:2:33:44:55
   Пробельные символы в начале и конце строки пропускаются. */
bool_t parse_mac(mac_t *mac, char *s)
{
  if (s == NULL)
  {
    fprintf(stderr, "parse_mac: s is NULL pointer.\n");
    return FALSE;
  }

  /* Пропускаем пробельные символы в начале строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Пытаемся разобрать MAC-адрес в формате 00:11:22:33:44:55 или 00-11-22-33-44-55 */
  char *p = parse_mac1(mac, s);
  if (p == NULL)
  {
    /* Пытаемся разобрать MAC-адрес в форматах 0011:2233:4455, 0011-2233-4455 или 0011.2233.4455 */
    p = parse_mac2(mac, s);
    if (p == NULL)
    {
      /* Пытаемся разобрать MAC-адрес в формате 001122:334455 или 001122-334455 */
      p = parse_mac3(mac, s);
      if (p == NULL)
      {
        /* Пытаемся разобрать MAC-адрес в формате 001122334455 */
        p = parse_mac4(mac, s);
        if (p == NULL)
        {
          /* Пытаемся разобрать MAC-адрес в формате 0:1:2:33:44:55 (октеты без ведущих нулей) */
          p = parse_mac5(mac, s);
          if (p == NULL)
          {
            return FALSE;
          }
        }
      }
    }
  }
  s = p;

  /* Пропускаем пробельные символы в конце строки */
  while (isspace(*s))
  {
    s++;
  }

  /* Если после пропуска завершающих пробельных символов не достигнут конец строки,
     значит исходная строка имеет неправильный синтаксис */
  if (*s != '\0')
  {
    return FALSE;
  }

  return TRUE;
}

/* Сравнение двух MAC-адресов на равенство */
bool_t is_equal_macs(mac_t *mac1, mac_t *mac2)
{
   if (mac1 == NULL)
   {
     fprintf(stderr, "is_equal_macs: mac1 is NULL pointer\n");
     return FALSE;
   }

   if (mac2 == NULL)
   {
     fprintf(stderr, "is_equal_macs: mac2 is NULL pointer\n");
     return FALSE;
   }

   if ((mac1->a != mac2->a) || 
       (mac1->b != mac2->b) ||
       (mac1->c != mac2->c) ||
       (mac1->d != mac2->d) ||
       (mac1->e != mac2->e) ||
       (mac1->f != mac2->f))
   {
     return FALSE;
   }
   return TRUE;
}

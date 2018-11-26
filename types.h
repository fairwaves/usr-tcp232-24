#ifndef __TYPES__
#define __TYPES__

/* Булевый тип */
typedef enum bool_e
{
  FALSE = 0,
  TRUE = !FALSE
} bool_t;

/* Разбор строки с неотрицательным целым числом, значение которого не превышает max.
   Пробелы в начале и в конце строки пропускаются */
bool_t parse_unsigned(unsigned *result, char *s, unsigned max);

typedef unsigned bitrate_t;

/* Разбор строки с битовой скоростью. 
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_bitrate(bitrate_t *bitrate, char *s);

/* Режим проверки чётности последовательного порта */
typedef enum parity_e
{
  PARITY_WRONG,
  PARITY_DISABLED,
  PARITY_ODD,
  PARITY_EVEN,
  PARITY_MARK,
  PARITY_CLEAR,
} parity_t;

/* Разбор строки с форматом слова в виде 8N1 или 8-N-1. 
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_word_format(unsigned char *db, parity_t *p, unsigned char *sb, char *s);

/* Разбор строки с настройками последовательного порта в виде 9600/8N1 или 9600/8-N-1.
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_serial(bitrate_t *br, unsigned char *db, parity_t *p, unsigned char *sb, char *s);

typedef unsigned short port_t;

/* Разбор строки с номером порта.
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_port(port_t *port, char *s);

typedef struct ip_s
{
  unsigned char a;
  unsigned char b;
  unsigned char c;
  unsigned char d;
} ip_t;

/* Разбор строки с IP-адресом.
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_ip(ip_t *ip, char *s);

typedef struct mac_s
{
  unsigned char a;
  unsigned char b;
  unsigned char c;
  unsigned char d;
  unsigned char e;
  unsigned char f;
} mac_t;

/* Разбор строки с MAC-адресом. Принимается один из форматов:
   00:11:22:33:44:55
   00-11-22-33-44-55
   0011:2233:4455
   0011-2233-4455
   00112233445566
   0:1:2:33:44:55 
   Пробельные символы в начале и конце строки пропускаются */
bool_t parse_mac(mac_t *mac, char *s);

/* Сравнение двух MAC-адресов на равенство */
bool_t is_equal_macs(mac_t *mac1, mac_t *mac2);

#endif

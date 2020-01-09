#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include "types.h"
#include "proto.h"
#include "net.h"

/* Здесь будет храниться широковещательный адрес */
static struct sockaddr broadcast;
static socklen_t broadcastlen = 0;

/* Здесь будет храниться прослушиваемый адрес */
static struct sockaddr anycast;
static socklen_t anycastlen = 0;

/* Подготовить широковещательный адрес и прослушиваемый адрес,
   Открыть слушающий UDP-сокет для взаимодействия с преобразователями интерфейсов,
   привязать его к указанному сетевому интерфейсу,
   разрешить отправлять через сокет пакеты на широковещательный адрес,
   выставить таймаут ожидания.

   Хорошее введение в сетевые системные вызовы:
   http://masandilov.ru/network/guide_to_network_programming5 */
int create_socket(char *interface)
{
  if (interface == NULL)
  {
    fprintf(stderr, "create_socket: interface is NULL\n");
    return -1;
  }

  /* Если переменная с широковещательным адресом ещё не заполнена,
     заплоняем её */
  if (broadcastlen == 0)
  {
    /* Подсказки для выбора IP-адреса */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       /* Адрес IPv4 */
    hints.ai_socktype = SOCK_DGRAM;  /* Сокет для обмена данными без установки подключения */
    hints.ai_flags = AI_ADDRCONFIG;  /* Указанный IP-адрес */
    hints.ai_protocol = IPPROTO_UDP; /* UDP-сокет */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

   /* Запрашиваем список подходящих IP-адресов */
    struct addrinfo *result;
    if (getaddrinfo(BROADCAST_IP, CONVERTER_PORT, &hints, &result) != 0)
    {
      fprintf(stderr, "create_socket: failed to get broadcast address information\n");
      return -1;
    }

    /* Копируем результат */
    memcpy(&broadcast, result->ai_addr, result->ai_addrlen);
    broadcastlen = result->ai_addrlen;

    /* Освобождаем список результатов */
    freeaddrinfo(result);
  }

  /* Если переменная с прослушиваемым адресом ещё не заполнена,
     заполняем её */
  if (anycastlen == 0)
  {
    /* Подсказки для выбора IP-адреса */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       /* Адрес IPv4 */
    hints.ai_socktype = SOCK_DGRAM;  /* Сокет для обмена данными без установки подключения */
    hints.ai_flags = AI_PASSIVE;     /* Любой локальный IP-адрес */
    hints.ai_protocol = IPPROTO_UDP; /* UDP-сокет */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    /* Запрашиваем список подходящих IP-адресов */
    struct addrinfo *result;
    if (getaddrinfo(ANYCAST_IP, CONVERTER_PORT, &hints, &result) != 0)
    {
      fprintf(stderr, "create_socket: failed to get anycast address information\n");
      return -1;
    }

    /* Копируем результат */
    memcpy(&anycast, result->ai_addr, result->ai_addrlen);
    anycastlen = result->ai_addrlen;

    /* Освобождаем список результатов */
    freeaddrinfo(result);
  }

  /* Пытаемся открыть сокет */
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1)
  {
    fprintf(stderr, "create_socket: failed to create socket\n");
    return -1;
  }

  /* Пытаемся подготовить сокет для прослушивания */
  if (bind(sock, &anycast, anycastlen) == -1)
  {
    fprintf(stderr, "create_socket: failed to bind socket to anycast address\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "create_socket: warning, failed to close socket\n");
    }
    return -1;
  }

  /* Подсоединяем сокет к указанному сетевому интерфейсу */
  if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, interface, strlen(interface)) == -1)
  {
    fprintf(stderr, "create_socket: failed to bind socket to specified interface\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "create_socket: warning, failed to close socket\n");
    }
    return -1;
  }

  /* Включаем опцию, разрешающую отправлять данные по широковещательному адресу */
  int b = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &b, sizeof(b)) == -1)
  {
    fprintf(stderr, "create_socket: failed to enable permission for sending to broadcast\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "create_socket: warning, failed to close socket\n");
    }
    return -1;
  }

  /* Выставляем таймаут ожидания ответа 2 секунды */
  struct timeval timeout = {2, 0};
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
  {
    fprintf(stderr, "create_socket: failed to set socket receive timeout\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "create_socket: warning, failed to close socket\n");
    }
    return -1;
  }

  return sock;
}

/* Этот перехватчик сигнала ALRM будет использоваться для определения момента завершения обнаружения */
static int discover_timeout = 0;
void discover_sighandler(int signal)
{
  if (signal == SIGALRM)
  {
    discover_timeout = 1;
  }
}
static struct sigaction new_sa;
static struct sigaction old_sa;

/* Завершить обнаружение преобразователей интерфейсов */
void discover_stop()
{
  /* Восстанавливаем старый обработчик сигнала ALRM */
  sigaction(SIGALRM, &old_sa, NULL);

  /* Выставляем признак необходимости завершить обнаружение */
  discover_timeout = 1;
}

/* Начать обнаружение преобразователей интерфейсов */
bool_t discover_start(int sock)
{
  if (sock < 0)
  {
    fprintf(stderr, "discover_start: sock is invalid\n");
    return FALSE;
  }

  /* Отправляем по широковещательному адресу запрос на обнаружение преобразователей интерфейсов */
  if (sendto(sock, DISCOVER_REQUEST, sizeof(DISCOVER_REQUEST), 0, &broadcast, broadcastlen) == -1)
  {
    fprintf(stderr, "discover_start: failed to send discover request\n");
    if (close(sock) == -1)
    {
      fprintf(stderr, "discover_start: warning, failed to close socket\n");
    }
    return FALSE;
  }

  /* Готовим новый обработчик сигнала ALRM */
  new_sa.sa_handler = discover_sighandler;
  new_sa.sa_flags = SA_RESTART;

  /* Таймаут ожидания ответов ещё не достигнут */
  discover_timeout = 0;

  /* Устанавливаем новый обработчик и запоминаем прежний обработчик */
  sigaction(SIGALRM, &new_sa, &old_sa);

  /* Настраиваем будильник. При срабатывании будильника будет вызван обработчик сигнала ALRM,
     который установит переменную discover_timeout в 1. Это значение будет означать, что пора
     завершать обнаружение преобразователей интерфейсов */
  alarm(DISCOVER_TIMEOUT);

  return TRUE;
}

/* Обнаружение следующего преобразователя интерфейсов */
bool_t discover_next(int sock, discover_t *discovered)
{
  if (sock < 0)
  {
    fprintf(stderr, "discover_next: sock is invalid\n");
    return FALSE;
  }

  if (discovered == NULL)
  {
    fprintf(stderr, "discover_next: discovered is NULL\n");
    return FALSE;
  }

  /* Пока не достигнут таймаут обнаружения */
  unsigned char buf[BUF_SIZE];
  while (discover_timeout != 1)
  {
    /* Принимаем пакеты до тех пор, пока не попадётся не пустой пакет */
    ssize_t size = recv(sock, buf, BUF_SIZE, 0);
    if (size <= 0)
    {
      continue;
    }

    /* Если принят пакет размером с пакет обнаружения, то пропускаем его - скорее всего это
       отправленный нами же пакет, принятый обратно, т.к. он был отправлен широковещательно */
    if (size == sizeof(DISCOVER_REQUEST))
    {
      continue;
    }

    /* Повторяем приём пакетов, пока не попадётся осмысленный пакет */
    if (discover_unpack(discovered, buf, size) == FALSE)
    {
      fprintf(stderr, "discover_next: warning, failed to unpack discovered data\n");
      continue;
    }
    
    /* Пакет с осмысленным ответом принят, передаём извлечённые данные
       вызывающей стороне для обработки */
    return TRUE;
  }

  /* Достигнут таймаут обнаружения - возвращаем прежни обработчик сигнала ALRM */
  discover_stop();
  return FALSE;
}

/* Обнаружение преобразователя интерфейсов с указанным MAC-адресом */
bool_t search(int sock, mac_t *mac, discover_t *discovered)
{
  if (sock < 0)
  {
    fprintf(stderr, "search: sock is invalid\n");
    return FALSE;
  }

  if (mac == NULL)
  {
    fprintf(stderr, "search: mac is NULL pointer\n");
    return FALSE;
  }

  if (discovered == NULL)
  {
    fprintf(stderr, "search: discovered is NULL pointer\n");
    return FALSE;
  }

  /* Запускаем обнаружение преобразователей интерфейсов */
  if (discover_start(sock) == FALSE)
  {
    fprintf(stderr, "search: discover_start failed\n");
    return FALSE;
  }

  /* Ищем интересующий нас преобразователь интерфейсов */
  while (discover_next(sock, discovered) == TRUE)
  {
    /* Если найден преобразователь интерфейсов, возвращаем информацию о нём */
    if (is_equal_macs(&(discovered->mac), mac) == TRUE)
    {
      return TRUE;
    }
  }

  fprintf(stderr, "search: warning, failed to discover specified converter\n");
  return FALSE;
}

/* Выполняет запись указанных параметров в преобразователь интерфейсов и проверяет,
   что указанные параметры применились */
bool_t apply_params(int sock, mac_t *mac, const params_t *params)
{
  if (sock < 0)
  {
    fprintf(stderr, "apply_params: sock is invalid\n");
    return FALSE;
  }

  if (mac == NULL)
  {
    fprintf(stderr, "apply_params: mac is NULL\n");
    return FALSE;
  }

  if (params == NULL)
  {
    fprintf(stderr, "apply_params: init_params is NULL\n");
    return FALSE;
  }

  /* Готовим настройки преобразователя интерфейсов */
  writeparams_t wp;
  wp.mac = *mac;
  memcpy(&(wp.password), DEFAULT_PASSWORD, sizeof(wp.password));
  wp.params = *params;

  /* Формируем пакет настройки преобразователя интерфейсов */
  unsigned char buf[BUF_SIZE];
  ssize_t size = writeparams_pack(&wp, buf, BUF_SIZE);
  if (size == -1)
  {
    fprintf(stderr, "apply_params: failed to pack writeparams\n");
    return FALSE;
  }

  /* Отправляем по широковещательному адресу новые настройки преобразователя интерфейсов */
  if (sendto(sock, buf, size, 0, &broadcast, broadcastlen) == -1)
  {
    fprintf(stderr, "apply_params: failed to send writeparams request\n");
    return FALSE;
  }

  /* Ждём, когда преобразователь интерфейсов применит новые настройки */
  sleep(4);

  /* Обнаруживаем устройство, которому были отправлены новые настройки */
  discover_t discovered;
  if (search(sock, mac, &discovered) == FALSE)
  {
    fprintf(stderr, "apply_params: failed to search configured converter\n");
    return FALSE;
  }

  /* Сверяем настройки из обнаруженного с записанными настройками */
  if (params_equal(&discovered.params, &wp.params) == FALSE)
  {
    fprintf(stderr, "apply_params: failed to apply new params to converter\n");
    return FALSE;
  }
  fprintf(stderr, "params was changed.\n");

  return TRUE;
}

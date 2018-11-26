#ifndef __NET__
#define __NET__

#include "proto.h"

#define BROADCAST_IP "255.255.255.255"
#define ANYCAST_IP "0.0.0.0"
#define CONVERTER_PORT "1500"
#define DISCOVER_TIMEOUT 5
#define BACKLOG 32
#define BUF_SIZE 64

/* Подготовить широковещательный адрес и прослушиваемый адрес,
   открыть слушающий UDP-сокет для взаимодействия с преобразователями интерфейсов,
   привязать его к указанному сетевому интерфейсу,
   разрешить отправлять через сокет пакеты на широковещательный адрес,
   выставить таймаут ожидания.

   Хорошее введение в сетевые системные вызовы:
   http://masandilov.ru/network/guide_to_network_programming5 */

int create_socket(char *interface);

/* Начать обнаружение преобразователей интерфейсов */
bool_t discover_start(int sock);

/* Обнаружение следующего преобразователя интерфейсов */
bool_t discover_next(int sock, discover_t *discovered);

/* Завершить обнаружение преобразователей интерфейсов */
void discover_stop();

/* Обнаружение преобразователя интерфейсов с указанным MAC-адресом */
bool_t search(int sock, mac_t *mac, discover_t *discovered);

/* Выполняет запись указанных параметров в преобразователь интерфейсов и проверяет,
 *    что указанные параметры применились */
bool_t apply_params(int sock, mac_t *mac, const params_t *params);

#endif

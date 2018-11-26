#ifndef __ACTIONS__
#define __ACTIONS__

/* Вывод справки об использовании программы */
int help(char *executed_pathname);

/* Обнаружение преобразователей интерфейсов */
int discover(char *interface);

/* Замена текущих настроек преобразователя интерфейсов */
int config(char *interface, char *dest_mac, int nopts, char *vopts[]);

/* Изменение текущих настроек преобразователя интерфейсов */
int change(char *interface, char *dest_mac, int nopts, char *vopts[]);

#endif

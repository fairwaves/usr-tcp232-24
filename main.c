/*
   Программа для настройки преобразователей интерфейсов "USR-TCP232-24"
   производителя "Jinan USR IOT Technology Limited".

   (C) 2018 Владимир Ступин
*/

#include <stdio.h>
#include <string.h>
#include "actions.h"

/* Главная функция, вызывается при запуске программы */
int main(int carg, char *varg[])
{
  if ((carg == 3) && (strcmp(varg[1], "discover") == 0))
  {
    char *source_ip = varg[2];
    return discover(source_ip);
  }
  else if ((carg >= 4) && (strcmp(varg[1], "config") == 0))
  {
    char *source_ip = varg[2];
    char *dest_mac = varg[3];

    int nopts = carg - 4;
    char **vopts = &(varg[4]);
    
    return config(source_ip, dest_mac, nopts, vopts);
  }
  else if ((carg >= 4) && (strcmp(varg[1], "change") == 0))
  {
    char *source_ip = varg[2];
    char *dest_mac = varg[3];

    int nopts = carg - 4;
    char **vopts = &(varg[4]);
    
    return change(source_ip, dest_mac, nopts, vopts);
  }

  return help(varg[0]);
}

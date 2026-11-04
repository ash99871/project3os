#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Tabla con las syscalls
struct {
  char *name;
  int   num;
} syscall_table[] = {
  {"fork",    1},
  {"exit",    2},
  {"wait",    3},
  {"pipe",    4},
  {"read",    5},
  {"kill",    6},
  {"exec",    7},
  {"fstat",   8},
  {"chdir",   9},
  {"dup",     10},
  {"getpid",  11},
  {"sbrk",    12},
  {"sleep",   13},
  {"uptime",  14},
  {"open",    15},
  {"write",   16},
  {"mknod",   17},
  {"unlink",  18},
  {"link",    19},
  {"mkdir",   20},
  {"close",   21},
  {"trace",   22},
  {"sysinfo", 23},
};

int
name_to_num(char *name)
{
  int n = sizeof(syscall_table) / sizeof(syscall_table[0]);
  for (int i = 0; i < n; i++) {
    if (strcmp(syscall_table[i].name, name) == 0)
      return syscall_table[i].num;
  }
  return -1;
}

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "Uso: trace <nombre_syscall> [comando args...]\n");
    exit(1);
  }

  int num = name_to_num(argv[1]);
  if (num < 0) {
    fprintf(2, "trace: syscall desconocida '%s'\n", argv[1]);
    exit(1);
  }

  if (trace(1 << num) < 0) {
    fprintf(2, "trace: no se pudo activar el trace para %s\n", argv[1]);
    exit(1);
  }

  // Ejecutar el comando pasado como argumento (a partir de argv[2])
  exec(argv[2], argv + 2);
  
  fprintf(2, "trace: exec %s falló\n", argv[2]);
  exit(1);
}
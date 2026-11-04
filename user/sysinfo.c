#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: error al obtener la información del sistema\n"); // Imprime en stderr
    exit(1);
  }

  printf("Free Memory: %ld MB\n", info.freemem);
  printf("Used Pages: %ld\n", info.used_pages);
  printf("Available Pages: %ld\n", info.avail_pages);
  printf("Runnable Processes: %ld\n", info.nproc);

  exit(0);
}
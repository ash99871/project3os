#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "user/user.h"

int
main(void)
{
  // 1. ugetpid() debe coincidir con getpid()
  printf("getpid=%d ugetpid=%d\n", getpid(), ugetpid());
  if(getpid() != ugetpid())
    printf("FALLO: los pids no coinciden\n");

  // 2. el hijo debe tener su propio pid en su propia página
  int pid = fork();
  if(pid == 0){
    printf("hijo: getpid=%d ugetpid=%d\n", getpid(), ugetpid());
    // 3. escribir en USYSCALL debe matar al proceso
    *(int *)USYSCALL = 1234;
    printf("FALLO: la escritura no fue bloqueada\n");
    exit(1);
  }
  int st;
  wait(&st);
  printf("hijo terminó con estado %d (esperado: -1)\n", st);

  // ---------------------------------------------------------
  // 5. Prueba de pgaccess() (Parte C)
  // ---------------------------------------------------------
  printf("\n=== Probando pgaccess() ===\n");
  
  // Reservar 3 páginas (3 * 4096 bytes)
  char *buf = sbrk(3 * 4096);
  uint64 mask;

  // Accedemos a la página 0 (lectura) y a la página 2 (escritura).
  // La página 1 queda intacta (probando lazy allocation y ceros).
  volatile int dummy = buf[10];  // Toca página 0
  (void)dummy;
  buf[2 * 4096 + 10] = 1;        // Toca página 2

  // Primer llamado: Esperamos bit 0 y bit 2 encendidos (101 en binario -> 5)
  if (pgaccess(buf, 3, &mask) < 0) {
    printf("FALLO: pgaccess devolvio -1 inesperadamente\n");
  } else {
    if (mask != 5) {
      printf("FALLO: pgaccess mask esperada 5, se obtuvo %d\n", (int)mask);
    } else {
      printf("EXITO: pgaccess bitmask inicial correcto (%d)\n", (int)mask);
    }
  }

  // Segundo llamado: Como pgaccess debe limpiar el PTE_A, ahora todo debe ser 0
  if (pgaccess(buf, 3, &mask) < 0) {
    printf("FALLO: segundo llamado pgaccess devolvio -1\n");
  } else {
    if (mask != 0) {
      printf("FALLO: pgaccess no limpio los bits, mask = %d\n", (int)mask);
    } else {
      printf("EXITO: pgaccess limpio los bits PTE_A correctamente (mask = %d)\n", (int)mask);
    }
  }

  // Pruebas de restricciones (Errores esperados)
  if (pgaccess(buf + 10, 1, &mask) != -1)
    printf("FALLO: pgaccess no rechazo base desalineada\n");
  else
    printf("EXITO: rechazo base desalineada\n");

  if (pgaccess(buf, 65, &mask) != -1)
    printf("FALLO: pgaccess no rechazo npages > 64\n");
  else
    printf("EXITO: rechazo npages mayor a 64\n");
    
  if (pgaccess(buf + 100 * 4096, 1, &mask) != -1)
    printf("FALLO: pgaccess no rechazo base fuera de limites\n");
  else
    printf("EXITO: rechazo acceso fuera de limites (p->sz)\n");

  // 4. tabla de páginas
  vmprint();
  exit(0);
}

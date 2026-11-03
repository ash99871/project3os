#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "sysinfo.h"
 
uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}
 
uint64
sys_getpid(void)
{
  return myproc()->pid;
}
 
uint64
sys_fork(void)
{
  return kfork();
}
 
uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}
 
uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;
 
  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;
 
  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > USYSCALL)             // P3: antes TRAPFRAME
      return -1;
    myproc()->sz += n;
  }
  return addr;
}
 
uint64
sys_pause(void)
{
  int n;
  uint ticks0;
 
  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}
 
uint64
sys_kill(void)
{
  int pid;
 
  argint(0, &pid);
  return kkill(pid);
}
 
// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;
 
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
 //archivo modificado
uint64
sys_sysinfo(void)
{
  uint64 addr;
  struct sysinfo info;
  struct proc *p = myproc();
 
  //Obtener la dirección como parámetro desde el usuario
  argaddr(0, &addr);
 
  //información del kernel
  uint64 free_pages = count_free_pages();
  uint64 total_pages = (PHYSTOP - KERNBASE) / PGSIZE;
 
  info.avail_pages = free_pages;
  info.used_pages = total_pages - free_pages;
  //Convertir bytes (páginas * tamaño de página) a Megabytes
  info.freemem = (free_pages * PGSIZE) / (1024 * 1024);
  info.nproc = count_runnable_procs();
 
  //Pasar la estructura al espacio de dirección del proceso de usuario
  if (copyout(p->pagetable, p->sz, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
 
  return 0;
}
 
// P3: imprime la tabla de páginas del proceso que la invoca
uint64
sys_vmprint(void)
{
  vmprint(myproc()->pagetable);
  return 0;
}

// Implementación para la Parte C: pgaccess
uint64
sys_pgaccess(void)
{
  uint64 base;
  int npages;
  uint64 mask_addr;
  uint64 mask_val = 0;
  struct proc *p = myproc();

  // 1. Obtener los argumentos desde el espacio de usuario
  argaddr(0, &base);
  argint(1, &npages);
  argaddr(2, &mask_addr);

  // 2. Validaciones estrictas
  if (base % PGSIZE != 0) 
    return -1; // Fallo si no está alineada a página
    
  if (npages < 1 || npages > 64) 
    return -1; // Fallo si npages está fuera de rango
    
  if (base >= p->sz || base + npages * PGSIZE > p->sz) 
    return -1; // Fallo si se sale del tamaño de memoria del proceso

  // 3. Revisar el bit de acceso página por página
  for (int i = 0; i < npages; i++) {
    uint64 va = base + i * PGSIZE;
    
    // Consultar la PTE sin forzar asignación de memoria (alloc=0)
    pte_t *pte = walk(p->pagetable, va, 0); 
    
    // Si la PTE existe y es válida (ignora lazy pages no tocadas)
    if (pte != 0 && (*pte & PTE_V) != 0) {
      if ((*pte & PTE_A) != 0) {
        mask_val |= (1L << i); // Encender el bit i en la máscara
        *pte &= ~PTE_A;        // Limpiar el bit PTE_A del hardware
      }
    }
  }

  // 4. Copiar la máscara resultante de vuelta al usuario
  // Usamos 5 parámetros respetando la firma de tu defs.h: 
  // copyout(pagetable_t, uint64, uint64, char*, uint64)
  if (copyout(p->pagetable, p->sz, mask_addr, (char *)&mask_val, sizeof(mask_val)) < 0) {
    return -1;
  }

  return 0;
}

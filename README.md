## Proyecto 3 sistemas operativos: tablas de pagina en accion

**integrantes**: Ashley sofia robayo y Samuel Quintero Quintero

## objetivo

Se expandio el kernel de xv6 con tres mecanismos que dependen de la tabla de páginas Sv39 de cada proceso.

# Parte A
en la parte a se creo una pagina compartida USYSCALL de solo lectura y ugetpid() sin llamada al sistema. Los archivo principales fueron memlayout.h, proc.h proc.c, ulib.c y sysproc.c

**memlayout.h**: USYSCALL se define como TRAPFRAME - PGSIZE. La estructura struct usyscall { int pid; }; va dentro de #ifndef __ASSEMBLER__ ... #endif, porque este encabezado tambien lo incluye el codigo en ensamblador.

**allocproc()**: reserva una pagina física con kalloc(), la llena con el pid del proceso y guarda su direccion en un campo nuevo de struct proc, y si la reserva falla, se liberan los recursos ya obtenidos y devuelve un error.

**proc_pagetable()**: mapea la pagina en USYSCALL con PTE_R | PTE_U. El proceso en el sistema puede leerla, pero no escribirla asi que  si el mapeo falla, se deshacen los mapeos anteriores (trampolin y trapframe) antes de regresar.

**ugetpid() en user/ulib.c**: lee ((struct usyscall *)USYSCALL)->pid, sin ejecutar ecall.

**freeproc() y proc_freepagetable()**: desmapean USYSCALL con uvmunmap() y la pagina fisica del programa se libera con kfree() para no dejar fugas.

**limitaciones del heap**: growproc() y sys_sbrk() ahora comparan contra USYSCALL en lugar de TRAPFRAME, de modo que el heap no puede crecer sobre la nueva pagina.

# parte B
Se creo vmprint() el cual imprime una tabla de paginas en sus tres niveles. Se uso vm.c sysproc.c y defs.h


**vm.c**: sys_vmprint() llama a una funcion que imprime page table <pa> con la direccion fisica de la raiz para luego recorrer el arbol de forma recursiva, con el mismo esquema que freewalk().

**º**por cada PTE con PTE_V siempre se imprime una linea con un " .." por cada nivel de profundidad, y la direccion virtual donde empieza el rango que cubre la PTE, el valor de la PTE y la direccion fisica del programa. Todos los numeros se imprimen con un %p.

**º**la direccion virtual se reconstruye acumulando el indice de cada nivel: va = base + i *(con tamaños de 1gb para el nivel 2).

**º** las PTEs sin PTE_V es preferible que se omitan. Por eso las paginas de sbrklazy() que no se han tocado no aparecen en la solucion.

**sysproc.c**: implementa los manejadores de llamadas al sistema que no pertenecen a archivos ni exec (sys_getpid, sys_fork, sys_sbrk, sys_sleep, sys_kill, sys_uptime...). Cada sys_* lee sus argumentos con argint()/argaddr(), llama a la función del kernel que hace el trabajo y devuelve el resultado en a0.

**defs.h**: funciona como un indice de prototipos de todo el kernel, osea que declara las funciones de cada modulo (vm.c, proc.c, kalloc.c...) y las estructuras que se usan sin definirlas. Existe para que un .c pueda llamar funciones definidas en otro; por eso aquí solo se declara, no se implementa.

# Parte c

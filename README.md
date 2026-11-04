## Proyecto 3 sistemas operativos: tablas de pagina en accion

**integrantes**: Ashly sofia robayo y Samuel Quintero Quintero

## objetivo

Se expandio el kernel de xv6 con tres mecanismos que dependen de la tabla de páginas Sv39 de cada proceso.

# Parte A
en la parte a se creo una pagina compartida USYSCALL de solo lectura y ugetpid() sin llamada al sistema. Los archivo principales fueron memlayout.h, proc.h proc.c, ulib.c y sysproc.c

**USYSCALL**: direccion virtual donde el kernel mapea, en cada proceso, una pagina especial de solo lectura. Es TRAPFRAME - PGSIZE, la pagina justo debajo del trapframe, en la zona alta del espacio de direcciones. Es una constante de memlayout.h, no una función.

**memlayout.h**: USYSCALL se define como TRAPFRAME - PGSIZE. La estructura struct usyscall { int pid; }; va dentro de #ifndef __ASSEMBLER__ ... #endif, porque este encabezado tambien lo incluye el codigo en ensamblador.

**allocproc()**: reserva una pagina física con kalloc(), la llena con el pid del proceso y guarda su direccion en un campo nuevo de struct proc, y si la reserva falla, se liberan los recursos ya obtenidos y devuelve un error.

**proc_pagetable()**: mapea la pagina en USYSCALL con PTE_R | PTE_U. El proceso en el sistema puede leerla, pero no escribirla asi que  si el mapeo falla, se deshacen los mapeos anteriores (trampolin y trapframe) antes de regresar.

**ugetpid() en user/ulib.c**: lee ((struct usyscall *)USYSCALL)->pid, sin ejecutar ecall, devuelve el pid leyendo USYSCALL directamente, sin llamada al sistema. Por eso es más rapida que getpid().

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
Primero verifica que la direccion inicial este en dentro de 4096 bytes, que no mire más de 64 pags, y que no se exceda la memoria total del proceso.

**walk()** mira la tabla de páginas e ignora las paginas que no existen en memoria física. 
Luego por cada página se mira el bit PTE_A (el 6), luego lo limpia según la situacion y devuelve la máscara al usuario con copyout().

## Preguntas



## ¿Por qué la página de guarda no tiene PTE_U? ¿Qué pasa si un programa desborda la pila?  
porque la pila crece hacia direcciones menores, y la pagina de guarda esta justo debajo de ella. exec() la deja con PTE_U apagado (con uvmclear()). y asi, cualquier acceso desde modo usuario a esa pagina produce una excepcion de página, porque el hardware exige PTE_U para accesos de usuario.

## Ademas de getpid(), ¿qué otras llamadas se podrían acelerar con la pagina compartida y cuales no?
uptime(): en principio se podria acelerar en caso de que el kernel publica el contador ticks en una pagina de solo lectura y la actualiza en cada interrupción de reloj. Habria que mapear una pagina comun a todos los procesos, y el valor seria igual de exacto como la ultima interrupcion de reloj. Es informacion no sensible, asi que compartirla no filtra nada. En cambio no se pueden acelerar las que cambian estado del kernel, requieren privilegio o pueden bloquear al proceso como por ejemplo fork, exit, wait, kill, exec, pause, pipe etc......

## ¿Que riesgo habria si USYSCALL se mapeara con PTE_W?
Uno de los mayores riesgos es que procesos podrian modificar el contenido de una pagina que el kernel considera suya. En esta implementación el impacto directo es pequeño, porque la página es privada de cada proceso y solo guarda el pid.

## ¿Por que pgaccess() reporta 0 para una pagina de sbrklazy() no tocada, y por qué no debe asignarle memoria?
sbrklazy() solo aumenta p->sz. No reserva memoria física ni crea PTEs. Mientras el proceso no toque la página, no existe una PTE válida y el hardware nunca pudo encender PTE_A, porque PTE_A solo lo enciende al acceder a una página mapeada. Por tanto, "no accedida" es la respuesta correcta: 0.

Y no se debe asignar porque alteraria lo que se observa, podria forzar procesos a consumir mas memoria de la que deberian y porque una memoria no deberia de usar memoria en procesos que no la usen, y una consulta no es un uso










## Declaracion de uso con IA
Comandos en debian y gitbash, revision para encontrar errores

porque la funcion sbrklazy() solo aumenta p->sz. No reserva memoria fisica ni crea PTEs. Mientras el proceso no toque la página, no existe una PTE valida y el hardware nunca pudo encender PTE_A, porque PTE_A solo lo enciende al acceder a una página mapeada. Por tanto, "no accedida" es la respuesta correcta: 0.

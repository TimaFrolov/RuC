#ifndef vm_context_H
#define vm_context_H

#include <semaphore.h>
#include <stdio.h>

#include "Defs.h"

#include "th_static.h"
#include "uniprinter.h"
#include "uniscanner.h"

struct vm_context;
typedef struct vm_context vm_context;

typedef struct ruc_vm_thread_arg
{
    vm_context *context;
    void *      arg;
} ruc_vm_thread_arg;

// Определение глобальных переменных
typedef struct vm_context
{
    int                       g;
    int                       xx;
    int                       iniproc;
    int                       maxdisplg;
    int                       wasmain;
    int                       reprtab[MAXREPRTAB];
    int                       rp;
    int                       identab[MAXIDENTAB];
    int                       id;
    int                       modetab[MAXMODETAB];
    int                       md;
    int                       mem[MAXMEMSIZE];
    int                       functions[FUNCSIZE];
    int                       funcnum;
    int                       threads[NUMOFTHREADS]; //, curthread, upcurthread;
    ruc_vm_thread_arg         threadargs[NUMOFTHREADS];
    int                       procd;
    int                       iniprocs[INIPROSIZE];
    int                       base;
    int                       adinit;
    int                       NN;
    universal_scanner_options input_options;
    universal_printer_options output_options;
    universal_printer_options error_options;
    universal_printer_options miscout_options;
    sem_t *                   sempr, *semdeb;

    /* Threads */
    int             __countTh;
    ruc_thread_info __threads[__COUNT_TH];

    int              __countSem;
    sem_t *          __sems[__COUNT_SEM];
    pthread_rwlock_t __lock_t_create;
    pthread_rwlock_t __lock_t_sem_create;
} vm_context;

/**
 * Initialize RuC VM context
 *
 * @param context Uninitialized RuC context
 */
extern void vm_context_init(vm_context *context);

/**
 * Deinitialize RuC VM context
 *
 * @param context Initialized RuC context
 */
extern void vm_context_deinit(vm_context *context);

#endif

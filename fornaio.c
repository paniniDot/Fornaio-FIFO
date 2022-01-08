#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include "DBGpthread.h"
#include "printerror.h"

#define CLIENTI 10

int distributoreTicket = 1; /* primo ticket (lo 0) preso dal fornaio */
int turnoCorrente = 0;
int personeInCoda = 0;

pthread_mutex_t mutexDistributore;
pthread_mutex_t mutexTurno;
pthread_cond_t condFornaio;
pthread_cond_t condTurno;
pthread_cond_t condClienteServito;


void *fornaio( void *arg ) {

    char Flabel[128];
    sprintf( Flabel, "Fornaio" );

    while( 1 ) {
        DBGpthread_mutex_lock( &mutexDistributore, Flabel );
        while( personeInCoda == 0 ){
            printf( "%s: non ci sono persone in coda, pisolo\n", Flabel );
            DBGpthread_cond_wait( &condFornaio, &mutexDistributore, Flabel );
        }
        DBGpthread_mutex_unlock( &mutexDistributore, Flabel );
        DBGpthread_mutex_lock( &mutexTurno, Flabel );
        /* entrato qui c'è sicuramente qualcuno in coda */
        turnoCorrente++;
        printf( "%s: sono stato svegliato! c'è qualcuno in coda, incremento il turno corrente che diventa %d!\n", Flabel, turnoCorrente );
        DBGpthread_cond_broadcast( &condTurno, Flabel );
        /* serve il cliente */
        DBGpthread_cond_wait( &condClienteServito, &mutexTurno, Flabel );
        DBGpthread_mutex_unlock( &mutexTurno, Flabel );
    }
    pthread_exit( NULL );
}

void *cliente( void *arg ){

    char Clabel[128];
    int mioTicket;

    DBGpthread_mutex_lock( &mutexDistributore, Clabel );
    mioTicket = distributoreTicket++;
    sprintf( Clabel, "Cliente %" PRIiPTR " con ticket %d", ( intptr_t )arg, mioTicket );
    personeInCoda++;
    printf( "%s: si aggiunge alla coda che conta %d persone\n", Clabel, personeInCoda );
    if( personeInCoda == 1 ){
        printf( "%s: sveglia il fornaio\n", Clabel );
        DBGpthread_cond_signal( &condFornaio, Clabel ); 
    }
    DBGpthread_mutex_unlock( &mutexDistributore, Clabel );

    DBGpthread_mutex_lock( &mutexTurno, Clabel );
    while( mioTicket != turnoCorrente ){
        printf( "%s: non è il mio turno (turnoCorrente = %d)\n", Clabel, turnoCorrente );
        DBGpthread_cond_wait( &condTurno, &mutexTurno, Clabel );
    }
    DBGpthread_mutex_lock( &mutexDistributore, Clabel );
    personeInCoda--;
    DBGpthread_mutex_unlock( &mutexDistributore, Clabel );

    /* il fornaio mi ha svegliato ed è il mio turno, vengo servito */
    sleep( 1 );
    printf( "%s: è il mio turno, mi faccio servire dal fornaio\n", Clabel );
    DBGpthread_cond_signal( &condClienteServito, Clabel );
    DBGpthread_cond_broadcast( &condTurno, Clabel );
    DBGpthread_mutex_unlock( &mutexTurno, Clabel );
    pthread_exit( NULL );
}

int main( void ) {
    int rc;
    pthread_t th;
    intptr_t i;

    DBGpthread_mutex_init( &mutexDistributore, NULL, "main" );
    DBGpthread_mutex_init( &mutexTurno, NULL, "main" );
    DBGpthread_cond_init( &condFornaio, NULL, "main" );
    DBGpthread_cond_init( &condTurno, NULL, "main" );

    rc = pthread_create( &th, NULL, fornaio, NULL );
    if( rc ) {
        PrintERROR_andExit( rc, "Creazione fornaio" );
    }

    for( i=0; i<CLIENTI; i++ ){
        rc = pthread_create( &th, NULL, cliente, ( void *)i );
        if( rc ) {
            PrintERROR_andExit( rc, "Creazione fornaio" );
        }   
    } 

    pthread_exit( NULL );
    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "saucer.h"

static struct saucer_data * saucers;
static struct missile_data * missiles;
static struct winsize win;

static pthread_t launcher_id;
static pthread_t * saucer_id;
static pthread_t * missile_id;
static pthread_barrier_t global_barrier;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t newobj_mutex = PTHREAD_MUTEX_INITIALIZER;

static int nproc;
static int next_missile;
static int missiles_left;
static int escaped;
static int launcher_x;
static int saucer_rate;
static int score;
static int playing;

int
main(int argc, char * argv[])
{
    int i;
	int saucer_counter = 0;
	int next_saucer = 0;
	int next_missile = 0;
	int bar_cnt;

	/*initialize everything*/
	ioctl(0, TIOCGWINSZ, &win);
	saucers = malloc(MAX_SAUCERS*sizeof(struct saucer_data));
	missiles = malloc(MAX_MISSILES*sizeof(struct missile_data));
	saucer_id = malloc(MAX_SAUCERS*sizeof(pthread_t));
	missile_id = malloc(MAX_MISSILES*sizeof(pthread_t));
	nproc = get_nprocs();
	pthread_setconcurrency(nproc);
	missiles_left = START_MISSILES;
	launcher_x = win.ws_col/2;
	saucer_rate = START_SAUCER_RATE;
	score = 0;
	playing = 1;
	bar_cnt = 2 + MAX_SAUCERS + MAX_MISSILES;

	pthread_barrier_init(&global_barrier, NULL, bar_cnt);

	/*init threads*/
	pthread_create(&launcher_id, NULL, launcher_init, NULL);
    for (i = 0; i < MAX_SAUCERS; i ++) {
		pthread_create(&saucer_id[i], NULL, saucer_init, &saucers[i]);
    } 
    for (i = 0; i < MAX_MISSILES; i ++) {
		pthread_create(&missile_id[i], NULL, missile_init, &missiles[i]);
    } 

	/*start synchronization loop*/
	while(playing) {
		
		pthread_barrier_wait(&global_barrier);
	}	

	return 0;
}

void *
saucer_init(void * data) 
{
	struct saucer_data * self = (struct saucer_data *)data;
    
	while(playing) {
		pthread_barrier_wait(&global_barrier);
	}

	pthread_exit(0);
}

void *
launcher_init(void * data) 
{
	while(playing) {
		/*beginning of loop actions*/
		pthread_barrier_wait(&global_barrier);
	}
	pthread_exit(0);
}

void *
missile_init(void * data) 
{
	struct missile_data * self = (struct missile_data *)data;
	self->ttl = -1;
	while(playing) {
		pthread_barrier_wait(&global_barrier);
	}
	pthread_exit(0);
}

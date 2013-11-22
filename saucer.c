#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "saucer.h"

static struct saucer_data * saucers;
static struct missile_data * missiles;
static struct winsize win;

static pthread_t last_thread_id;
static pthread_barrier_t global_barrier;
static pthread_barrier_t creation_barrier;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int new_objs;
static int nproc;
static int new_missile_ind;
static int missiles_left;
static int escaped;
static int launcher_x;
static int saucer_rate;
static int score;
static int playing;

int
main(int argc, char * argv[])
{
	int saucer_counter = 0;
	int next_saucer = 0;
	int next_missile = 0;
	int bar_cnt;
	char * drawbuf;

	/*initialize everything*/
	ioctl(0, TIOCGWINSZ, &win);
	drawbuf = malloc(win.ws_row * win.ws_col * sizeof(char));
	saucers = malloc(1024*sizeof(struct saucer_data));
	missiles = malloc(1024*sizeof(struct missile_data));
	nproc = get_nprocs();
	pthread_setconcurrency(nproc);
	missiles_left = START_MISSILES;
	launcher_x = win.ws_col/2;
	saucer_rate = START_SAUCER_RATE;
	score = 0;
	playing = 1;
	bar_cnt = 2;

	pthread_barrier_init(&global_barrier, NULL, bar_cnt); 

	/*init launcher thread*/
	pthread_create(&last_thread_id, NULL, launcher_init, NULL);

	/*start synchronization loop*/
	while(playing) {
		/*reinitialized barriers if objects created*/
		if (new_objs) {
			printf("start\n");
			bar_cnt += new_objs;
			pthread_barrier_destroy(&global_barrier);
			pthread_barrier_init(&global_barrier, NULL, bar_cnt);
			pthread_barrier_wait(&creation_barrier);
			printf("middle\n");
			pthread_barrier_destroy(&creation_barrier);
			pthread_barrier_init(&creation_barrier, NULL, bar_cnt);
			pthread_barrier_wait(&global_barrier);
			new_objs = 0;
			printf("end\n");
		}
		/*perform action on any per loop global variables*/
		new_missile_ind = -1;
		saucer_counter = (saucer_counter + 1) % saucer_rate;
		
		/*Spawn saucers*/
		if (saucer_counter == 0) {
			/*TODO: check if reallocation is needed*/
			pthread_create(&last_thread_id, NULL, 
				saucer_init, &saucers[next_saucer]);
			next_saucer ++;
			pthread_mutex_lock(&mutex);
			new_objs ++;
			pthread_mutex_unlock(&mutex);
		}
		pthread_barrier_wait(&global_barrier);

		/*wait for movement of missles and saucers*/
		pthread_barrier_wait(&global_barrier);

		/*wait for player interaction*/
		pthread_barrier_wait(&global_barrier);

		/*collision checks if missile spawned*/
		/*put sprites into draw buffer*/
		pthread_barrier_wait(&global_barrier);

		/*draw everything to the screen*/
		/*wait for next timer step*/
		pthread_barrier_wait(&global_barrier);
	}	

	return 0;
}

void *
launcher_init(void * data) 
{
	while(playing) {
		/*beginning of loop actions*/
		if (new_objs) {
			pthread_barrier_wait(&creation_barrier);
			pthread_barrier_wait(&global_barrier);
		}
		pthread_barrier_wait(&global_barrier);

		/*wait for saucers and missiles to move*/
		pthread_barrier_wait(&global_barrier);

		/*TODO:perform player interactions*/
		pthread_barrier_wait(&global_barrier);

		/*wait for collision checks*/
		/*Wait for sprites to be put into draw buffer*/
		pthread_barrier_wait(&global_barrier);

		/*wait for end of loop actions*/
		pthread_barrier_wait(&global_barrier);
	}
	pthread_exit(0);
}

void *
saucer_init(void * data) 
{
	struct saucer_data * self = (struct saucer_data *)data;
	self->ttl = -1;

	pthread_barrier_wait(&creation_barrier);
	pthread_barrier_wait(&global_barrier);

	while(playing) {
		/*beginning of loop actions*/
		if (new_objs) {
			pthread_barrier_wait(&creation_barrier);
			pthread_barrier_wait(&global_barrier);
		}
		if (self->ttl > 0) {
			self->ttl --;
		}
		pthread_barrier_wait(&global_barrier);

		/*move self*/
		self->x += self->speed;
		pthread_barrier_wait(&global_barrier);

		/*wait for player interactions*/
		pthread_barrier_wait(&global_barrier);

		/*TODO:check for collisions if necessary*/
		pthread_barrier_wait(&global_barrier);

		/*wait for end of loop actions*/
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
		/*beginning of loop actions*/
		if (new_objs) {
			pthread_barrier_wait(&creation_barrier);
			pthread_barrier_wait(&global_barrier);
		}
		if (self->ttl > 0) {
			self->ttl --;
		}
		pthread_barrier_wait(&global_barrier);

		/*move self*/
		self->y -= MISSILE_SPEED;
		pthread_barrier_wait(&global_barrier);

		/*perform player interactions*/
		pthread_barrier_wait(&global_barrier);

		/*wait for collision checks*/ 
		pthread_barrier_wait(&global_barrier);

		/*wait for end of loop actions*/
		pthread_barrier_wait(&global_barrier);
		
	}
	pthread_exit(0);
}

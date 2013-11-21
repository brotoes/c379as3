#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "saucer.h"

static struct saucer_data * saucers;
static struct missile_data * missiles;
static pthread_t last_thread_id;
static pthread_barrier_t * global_barrier;

int nproc;
int new_missile_ind;
int missiles;
int escaped;
int launcher_x;
int saucer_rate;
int score;
struct winsize win;	

int
main(int argc, char * argv[])
{
	int saucer_counter = 0;
	int playing = 1;
	char * drawbuf;

	/*initialize everything*/
	ioctl(0, TIOCGWINSZ, &win);
	drawbuf = malloc(win.ws_row * win.ws_col * sizeof(char));
	nproc = get_nprocs();
	pthread_setconcurrency(nproc);
	missiles = START_MISSILES;
	launcher_x = win.ws_col/2;
	saucer_rate = START_SAUCER_RATE;
	score = 0;

	/*init launcher thread*/
	pthread_create(&last_thread_id, NULL, launcher_init, NULL);

	/*start synchronization loop*/
	while(playing) {
		/*perform action on any per loop global variables*/
		new_missile_ind = -1;
		saucer_counter = (saucer_counter + 1) % saucer_rate;
		/*Spawn saucers*/
		pthread_barrier_wait(global_barrier);

		/*wait for movement of missles and saucers*/
		pthread_barrier_wait(global_barrier);

		/*wait for player interaction*/
		pthread_barrier_wait(global_barrier);

		/*collision checks if missile spawned*/
		/*put sprites into draw buffer*/
		pthread_barrier_wait(global_barrier);

		/*draw everything to the screen*/
		/*wait for next timer step*/
		pthread_barrier_wait(global_barrier);
	}	

	return 0;
}

void *
launcher_init(void * data) 
{
	/*wait for saucers to spawn*/
	pthread_barrier_wait(global_barrier);

	/*wait for saucers and missiles to move*/
	pthread_barrier_wait(global_barrier);

	/*TODO:perform player interactions*/
	pthread_barrier_wait(global_barrier);

	/*wait for collision checks*/
	/*Wait for sprites to be put into draw buffer*/
	pthread_barrier_wait(global_barrier);

	/*wait for end of loop actions*/
	pthread_barrier_wait(global_barrier);
}

void *
saucer_init(void * data) 
{
	/*perform actions of per loop local variables*/
	data.ttl --;
	pthread_barrier_wait(global_barrier);

	/*move self*/
	data.x += data.speed;
	pthread_barrier_wait(global_barrier);

	/*wait for player interactions*/
	pthread_barrier_wait(global_barrier);

	/*TODO:check for collisions if necessary*/
	pthread_barrier_wait(global_barrier);

	/*wait for end of loop actions*/
	pthread_barrier_wait(global_barrier);
}

void *
missile_init(void * data) 
{
	/*perform actions on per loop local variables*/
	data.ttl --;
	pthread_barrier_wait(global_barrier);

	/*move self*/
	data.y -= MISSILE_SPEED;
	pthread_barrier_wait(global_barrier);

	/*perform player interactions*/
	pthread_barrier_wait(global_barrier);

	/*wait for collision checks*/ 
	pthread_barrier_wait(global_barrier);

	/*wait for end of loop actions*/
	pthread_barrier_wait(global_barrier);
	
}

#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <pthread.h>
#include <ioctl.h>
#include "saucer.h"

static struct saucer_data * saucers;
static struct missile_data * missiles;
static struct drawer_data to_draw;
static pthread_t * thread_ids;
static pthread_barrier_t * global_barrier;

int nproc;
int new_missile_ind;
int missiles;
int escaped;

int
main(int argc, char * argv[])
{
	int playing = 1;
	char * drawbuf;
	struct winsize win;	

	/*initialize everything*/
	ioctl(0, TIOCGWINSZ, &win);
	drawbuf = malloc(win.ws_row * win.ws_col * sizeof(char));
	nproc = get_nprocs();
	pthread_setconcurrency(nproc);

	/*init launcher thread*/
	pthread_create(thread_ids, );

	/*start synchronization loop*/
	while(playing) {
		/*reset any per loop global variables*/
		new_missile_ind = -1;
		/*Spawn saucers*/
		pthread_barrier_wait(global_barrier);
		/*movement of missles and saucers*/
		pthread_barrier_wait(global_barrier);
		/*player interaction*/
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
launcher_init(void * data) {
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
	/*wait for saucers to spawn*/
	pthread_barrier_wait(global_barrier);
	/*TODO:move self*/
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
	/*wait for saucers to spawn*/
	pthread_barrier_wait(global_barrier);
	/*TODO:move self*/
	pthread_barrier_wait(global_barrier);
	/*perform player interactions*/
	pthread_barrier_wait(global_barrier);
	/*wait for collision checks*/ 
	pthread_barrier_wait(global_barrier);
	/*wait for end of loop actions*/
	pthread_barrier_wait(global_barrier);
	
}

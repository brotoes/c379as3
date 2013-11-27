#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include "saucer.h"

static struct saucer_data * saucers;
static struct rocket_data * rockets;
static struct launcher_data launcher;
static pthread_t launcher_id;
static pthread_t * saucer_id;
static pthread_t * rocket_id;
static pthread_barrier_t substep_bar;
static pthread_barrier_t step_bar;
static pthread_mutex_t mutex;

static int cols;
static int lines;
static int nproc;
static int next_rocket;
static int next_saucer;
static int escaped;
static int destroyed;
static int saucer_rate;
static int score;
static int playing;
static int loop_cnt;
static int combo;
static int max_combo;
static int killed;

static void
killhandler()
{
	killed = 1;
}

int
main(int argc, char * argv[])
{
	char datadisp[80];
	int i;
	int j;
	int saucer_counter = 0;
	int next_saucer = 0;
	int next_rocket = 0;
	const int thread_cnt = 2 + MAX_SAUCERS + MAX_ROCKETS;
	struct sigaction sa;

	/*handle signals*/
	sa.sa_handler = killhandler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		err(1, "sigaction failed\n");	
	}

	/*initialize variables*/
	saucers = malloc(MAX_SAUCERS*sizeof(struct saucer_data));
	rockets = malloc(MAX_ROCKETS*sizeof(struct rocket_data));
	saucer_id = malloc(MAX_SAUCERS*sizeof(pthread_t));
	rocket_id = malloc(MAX_ROCKETS*sizeof(pthread_t));
	nproc = get_nprocs();
	pthread_setconcurrency(nproc);
	saucer_rate = START_SAUCER_RATE;
	next_saucer = 0;
	next_rocket = 0;
	score = 0;
	destroyed = 0;
	escaped = 0;
	playing = 1;
	loop_cnt = 0;
	max_combo = 1;
	combo = 1;
	killed = 0;
	srand(time(NULL));

	/*initialize pthreads*/
	pthread_barrier_init(&substep_bar, NULL, thread_cnt);
	pthread_barrier_init(&step_bar, NULL, thread_cnt);

	if (pthread_create(&launcher_id, NULL, launcher_init, &launcher)) {
		err(1, "Error creating thread\n");
	}
	for (i = 0; i < MAX_SAUCERS; i ++) {
		if (pthread_create(&saucer_id[i], NULL, saucer_init, &saucers[i])) {
			err(1, "Error creating thread\n");
		}
	}
	for (i = 0; i < MAX_ROCKETS; i ++) {
		if (pthread_create(&rocket_id[i], NULL, rocket_init, &rockets[i])) {
			err(1, "Error creating thread\n");
		}
	}

	/*initialize ncurses*/
	initscr();
	crmode();
	noecho();
	clear();
	nodelay(stdscr, true);
	cols = COLS;
	lines = LINES;

	/*start synchronization loop*/
	while(playing) {
		/*Spawn Saucers*/
		if (rand() % (saucer_rate*2) == 0) {
			saucers[next_saucer].speed = rand() % (SAUCER_SPEED_TOP -
				SAUCER_SPEED_LOW + 1) + SAUCER_SPEED_LOW;
			saucers[next_saucer].x = 0;
			saucers[next_saucer].state = STATE_LIVE;
			saucers[next_saucer].y = rand() % (SAUCER_REGION_TOP -
				SAUCER_REGION_BOT + 1) + SAUCER_REGION_BOT;
			next_saucer = (next_saucer + 1) % MAX_SAUCERS;
		}
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&substep_bar);

		/*Draw Everything*/
		clear();

		sprintf(datadisp, 
			"[Rockets: %5d]  [Destroyed: %5d]  \
[Escaped: %3d/%3d]  [Score:%d %d]",
			launcher.rockets_left, destroyed, 
			escaped, MAX_ESCAPE, score, combo);
		move(0,0);

		/*draw saucers*/
		for (i = 0; i < MAX_SAUCERS; i ++) {
			if (saucers[i].state == STATE_LIVE) {
				move(saucers[i].y/PRECISION, 
					saucers[i].x/PRECISION);
				addstr("<--->");
			}
			if (saucers[i].state == STATE_FALLING) {
				move(saucers[i].y/PRECISION, 
					saucers[i].x/PRECISION);
				addstr("<***>");
				for (j = 0; j < 3; j ++) {
					move((saucers[i].y - 10 -
						(loop_cnt+j)%30)/PRECISION, 
						(saucers[i].x+(loop_cnt+j)%30)
						/PRECISION + 2);
					switch (rand()%3) {
						case 0:
						case 1: {
							addch('*');
						}break;
						case 2: {
							addch('.');	
						}

					}
				}
			}
		}
		/*draw rockets*/
		for (i = 0; i < MAX_ROCKETS; i ++) {
			if (rockets[i].state == STATE_LIVE) {
				move(rockets[i].y/PRECISION,
					 rockets[i].x/PRECISION);
				addch('^');
				for (j = 0; j < 2; j ++) {
					move((rockets[i].y + 10 +
						(loop_cnt+j*20)%30)/PRECISION,
						(rockets[i].x)/PRECISION);
					addch('.');
				}
			}
		}
		/*draw launcher*/
		move(lines - 2, launcher.x/PRECISION);
		addch('|');
		move(lines - 1, 0);
		/*draw data*/
		addstr(datadisp);
		move(lines - 1, cols - 1);
		refresh();

		/*Iterate Per-loop variables*/
		loop_cnt ++;
		/*Check End Game Conditions*/
		if (launcher.rockets_left == 0 || escaped >= MAX_ESCAPE) {
			int rocket_onscreen = 0;
			for (i = 0; i < MAX_ROCKETS; i ++) {
				rocket_onscreen += rockets[i].state;
			}
			if (!rocket_onscreen) {
				playing = 0;	
			}
		}
		/*Wait for next game step*/
		if (playing) {
			usleep(1000000/GAME_SPEED);
			pthread_barrier_wait(&step_bar);
		}
		if (killed) {
			/*terminate cleanly and print score*/
			endwin();
			printf("GAME LEFT...\n");
			printf("SCORE: %20d\n", score);
			printf("HIGHEST COMBO: %12d\n", max_combo);
			printf("SAUCERS DESTROYED: %8d\n", destroyed);
			return 0;
		}
	}

	/*exit and print score*/
	endwin();
	printf("GAME OVER...\n");
	printf("SCORE: %20d\n", score);
	printf("HIGHEST COMBO: %12d\n", max_combo);
	printf("SAUCERS DESTROYED: %8d\n", destroyed);
	return 0;
}

static void *
launcher_init(void * data)
{
	/*rocket queued to be fired*/
	int waiting = 0;
	/*time until next rocket can be fired*/
	int delay = 0;
	/*store input*/
	int inputc;
	/*cast input argument*/
	struct launcher_data * self = (struct launcher_data *) data;

	self->rockets_left = START_ROCKETS;
	self->x = -1;

	while(playing) {
		/*handle input*/
		inputc = getch();
		flushinp();
		if (inputc == 'a') {
			/*move left*/
			if (self->x > 0) {
				self->x -= PRECISION;
			}
		} else if (inputc == 'd') {
			/*move right*/
			if (self->x < (cols - 1)*PRECISION) {
				self->x += PRECISION;
			}
		} else if (inputc == ' ') {
			/*launch rocket*/
			if (self->rockets_left) {
				waiting = 1;
			}
		}
		if (waiting && !delay) {
			rockets[next_rocket].x = self->x;
			rockets[next_rocket].y = (lines - 2)*PRECISION;
			rockets[next_rocket].state = STATE_INIT;
			next_rocket = (next_rocket + 1 ) % MAX_ROCKETS;
			self->rockets_left --;
			waiting = 0;
			delay = ROCKET_DELAY;
		}
		if (delay) {
			delay --;
		}
		pthread_barrier_wait(&substep_bar);

		/*Initial x position*/
		if (self->x == -1) {
			self->x = PRECISION*cols/2;
		}
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&step_bar);
	}

	pthread_exit(0);
}

static void *
saucer_init(void * data)
{
	struct saucer_data * self = (struct saucer_data *)data;

	self->state = STATE_DEAD;

	while(playing) {
		if (self->state == STATE_LIVE) {
			/*move*/
			self->x += self->speed;
			/*check room bounds*/
			if (self->x > (cols - 6)*PRECISION) {
				self->state = STATE_DEAD;
				pthread_mutex_lock(&mutex);
				combo = 1;
				escaped ++;
				pthread_mutex_unlock(&mutex);
			}
		}
		if (self->state == STATE_FALLING) {
			self->y += FALL_SPEED;
			self->x += self->speed/2;
			if (self->y > (lines-2)*PRECISION ||
				loop_cnt % (FALL_TIME*2) == 0) {
				self->state = STATE_DEAD;
			}
		}
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&substep_bar);

		/*Check and set state*/
		if (self->state == STATE_INIT) {
			self->state = STATE_LIVE;
		}
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&step_bar);
	}

	pthread_exit(0);
}

static void *
rocket_init(void * data)
{
	struct rocket_data * self = (struct rocket_data *)data;
	int lowest_hit;
	int i;

	self->state = STATE_DEAD;

	while(playing) {
		/*Move*/
		self->y -= ROCKET_SPEED;
		/*Test Room Boundaries*/
		if (self->y <= 0 && self->state == STATE_LIVE) {
			self->state = STATE_DEAD;
			pthread_mutex_lock(&mutex);
			combo = 1;
			pthread_mutex_unlock(&mutex);
		}
		pthread_barrier_wait(&substep_bar);

		/*Check Collisions*/
		lowest_hit = -1;
		for (i = 0; i < MAX_SAUCERS; i ++) {
			/*Check horizontal collision*/
			if (saucers[i].state == STATE_LIVE &&
				saucers[i].x/PRECISION <= self->x/PRECISION &&
				saucers[i].x/PRECISION + 4 > self->x/PRECISION) {
				/*Check vertical collision*/
				if (saucers[i].y/PRECISION <= 
					self->y/PRECISION + ROCKET_SPEED &&
					saucers[i].y/PRECISION >= self->y/PRECISION) {
					/*check if the saucer is below another
					 *already hit saucer*/
					if (lowest_hit == -1 || saucers[i].y <
						saucers[lowest_hit].y) {
						lowest_hit = i;
					}
				}
			}
		}
		if (lowest_hit >= 0 && self->state == STATE_LIVE) {
			pthread_mutex_lock(&mutex);
			saucers[lowest_hit].state = STATE_FALLING;
			self->state = STATE_DEAD;
			score += combo * (lines + 10 * saucers[lowest_hit].speed);
			combo ++;
			if (combo > max_combo) {
				max_combo = combo;	
			}
			destroyed ++;
			saucer_rate -= RATE_INCREASE;
			if (saucer_rate < MIN_RATE) {
				saucer_rate = MIN_RATE;	
			}
			launcher.rockets_left += combo;
			pthread_mutex_unlock(&mutex);
		}
		pthread_barrier_wait(&substep_bar);

		/*Check and change state*/
		if (self->state == STATE_INIT) {
			self->state = STATE_LIVE;
		}
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
		pthread_barrier_wait(&step_bar);
	}

	pthread_exit(0);
}

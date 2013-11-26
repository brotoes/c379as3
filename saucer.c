#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <curses.h>
#include <signal.h>
#include "saucer.h"

static struct saucer_data * saucers;
static struct missile_data * missiles;
static struct launcher_data launcher;
static pthread_t launcher_id;
static pthread_t * saucer_id;
static pthread_t * missile_id;
static pthread_barrier_t substep_bar;
static pthread_barrier_t step_bar;
static pthread_mutex_t mutex;

static int cols;
static int lines;
static int nproc;
static int next_missile;
static int next_saucer;
static int escaped;
static int destroyed;
static int saucer_rate;
static int score;
static int playing;
static int loop_cnt;
static int combo;
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
	int saucer_counter = 0;
	int next_saucer = 0;
	int next_missile = 0;
	const int thread_cnt = 2 + MAX_SAUCERS + MAX_MISSILES;
	struct sigaction sa;

	/*handle signals*/
	sa.sa_handler = killhandler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		err(1, "sigaction failed\n");	
	}

	/*initialize variables*/
	saucers = malloc(MAX_SAUCERS*sizeof(struct saucer_data));
	missiles = malloc(MAX_MISSILES*sizeof(struct missile_data));
	saucer_id = malloc(MAX_SAUCERS*sizeof(pthread_t));
	missile_id = malloc(MAX_MISSILES*sizeof(pthread_t));
	nproc = get_nprocs();
	pthread_setconcurrency(nproc);
	saucer_rate = START_SAUCER_RATE;
	next_saucer = 0;
	next_missile = 0;
	score = 0;
	destroyed = 0;
	escaped = 0;
	playing = 1;
	loop_cnt = 0;
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
	for (i = 0; i < MAX_MISSILES; i ++) {
		if (pthread_create(&missile_id[i], NULL, missile_init, &missiles[i])) {
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
		if (loop_cnt % (saucer_rate*GAME_SPEED) == 0) {
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

		sprintf(datadisp, "[Missiles: %3d]  [Destroyed: %3d]  [Escaped: %3d/%3d]  [Score:%d %d]",
			launcher.missiles_left, destroyed, escaped, MAX_ESCAPE, score, combo);
		move(0,0);

		for (i = 0; i < MAX_SAUCERS; i ++) {
			if (saucers[i].state == STATE_LIVE) {
				move(saucers[i].y/PRECISION, saucers[i].x/PRECISION);
				addstr("<--->");
			}
		}
		for (i = 0; i < MAX_MISSILES; i ++) {
			if (missiles[i].state == STATE_LIVE) {
				move(missiles[i].y/PRECISION, missiles[i].x/PRECISION);
				addch('^');
			}
		}
		move(lines - 2, launcher.x/PRECISION);
		addch('|');
		move(lines - 1, 0);
		addstr(datadisp);
		move(lines - 1, cols - 1);
		refresh();

		/*Iterate Per-loop variables*/
		loop_cnt ++;
		/*Check End Game Conditions*/
		if (launcher.missiles_left == 0 || escaped >= MAX_ESCAPE) {
			int missile_onscreen = 0;
			for (i = 0; i < MAX_MISSILES; i ++) {
				missile_onscreen += missiles[i].state;
			}
			if (!missile_onscreen) {
				playing = 0;	
			}
		}
		/*Wait for next game step*/
		if (playing) {
			usleep(1000000/GAME_SPEED);
			pthread_barrier_wait(&step_bar);
		}
		if (killed) {
			endwin();
			printf("GAME LEFT...\n");
			printf("SCORE: %9d\n", score);
			return 0;
		}
	}

	endwin();
	printf("GAME OVER...\n");
	printf("SCORE: %9d\n", score);
	return 0;
}

static void *
launcher_init(void * data)
{
	/*missile queued to be fired*/
	int waiting = 0;
	/*time until next missile can be fired*/
	int delay = 0;
	/*store input*/
	int inputc;
	/*cast input argument*/
	struct launcher_data * self = (struct launcher_data *) data;

	self->x = 800;
	self->missiles_left = START_MISSILES;

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
			/*launch missile*/
			if (self->missiles_left) {
				waiting = 1;
			}
		}
		if (waiting && !delay) {
			missiles[next_missile].x = self->x;
			missiles[next_missile].y = (lines - 2)*PRECISION;
			missiles[next_missile].state = STATE_INIT;
			next_missile = (next_missile + 1 ) % MAX_MISSILES;
			self->missiles_left --;
			waiting = 0;
			delay = MISSILE_DELAY;
		}
		if (delay) {
			delay --;
		}
		pthread_barrier_wait(&substep_bar);

		/*Do Nothing*/
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
			if (self->x > (cols - 1)*PRECISION
				&& self->state == STATE_LIVE) {
				self->state = STATE_DEAD;
				pthread_mutex_lock(&mutex);
				combo = 1;
				escaped ++;
				pthread_mutex_unlock(&mutex);
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
missile_init(void * data)
{
	struct missile_data * self = (struct missile_data *)data;
	int lowest_hit;
	int i;

	self->state = STATE_DEAD;

	while(playing) {
		/*Move*/
		self->y -= MISSILE_SPEED;
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
				saucers[i].x <= self->x &&
				saucers[i].x + 5*PRECISION >= self->x) {
				/*Check vertical collision*/
				if (saucers[i].y >= self->y - MISSILE_SPEED &&
					saucers[i].y <= self->y) {
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
			saucers[lowest_hit].state = STATE_DEAD;
			self->state = STATE_DEAD;
			score += combo * (100 + 10 * saucers[lowest_hit].speed);
			combo ++;
			destroyed ++;
			launcher.missiles_left += MISSILE_REWARD;
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

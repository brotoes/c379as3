#ifndef SAUCER_H
#define SAUCER_H

#define MAX_SAUCERS 30		/*Number of saucers on scren max*/
#define MAX_ROCKETS 30		/*Number of rockets on screen max*/
#define SAUCER_REGION_TOP 10	/*Region Saucers spawn within*/
#define SAUCER_REGION_BOT 50	
#define ROCKET_SPEED 5		/*Speed rockets move at*/
#define SAUCER_SPEED_TOP 5	/*range of speed saucers can have*/
#define SAUCER_SPEED_LOW 1	/*	Change of x per game step*/
#define START_ROCKETS 10	/*How many rockets the player starts with*/
#define START_SAUCER_RATE 100	/*average steps between saucer spawns*/
#define GAME_SPEED 30 		/*Number of steps the game takes per second*/
#define PRECISION 10		/*coordinate values within a col or row*/
#define ROCKET_DELAY 10		/*number of steps between rocket launches*/
#define MAX_ESCAPE 10		/*number of escaped allowed before game over*/
#define RATE_INCREASE 1		/*How much is subtracted from saucer_rate
				 *Upon killing a saucer*/
#define MIN_RATE 30		/*Fastest saucers may spawn*/
#define FALL_SPEED 2		/*Speed a saucer falls*/
#define FALL_TIME 60		/*number of steps a dead saucer falls*/
				 
#define STATE_LIVE 1
#define STATE_DEAD 0
#define STATE_INIT 3
#define STATE_HIT 2
#define STATE_FALLING 4

struct launcher_data {
	int x;
	int rockets_left;
};

struct rocket_data {
	int x;
	int y;
	int state;
};

struct saucer_data {
	int x;
	int y;
	int state;
	int speed;
	int death_time;
};

static void killhandler();

static void * launcher_init(void *);
static void * saucer_init(void *);
static void * rocket_init(void *);

#endif

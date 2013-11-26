#ifndef SAUCER_H
#define SAUCER_H

#define MAX_SAUCERS 30		/*Number of saucers on scren max*/
#define MAX_MISSILES 30		/*Number of missiles on screen max*/
#define SAUCER_REGION_TOP 10	/*Region Saucers spawn within*/
#define SAUCER_REGION_BOT 80	
#define MISSILE_SPEED 5		/*Speed missiles move at*/
#define SAUCER_SPEED_TOP 5	/*range of speed saucers can have*/
#define SAUCER_SPEED_LOW 1	/*	Change of x per game step*/
#define START_MISSILES 10	/*How many missiles the player starts with*/
#define START_SAUCER_RATE 5 	/*Number of seconds between saucer spawns*/
#define GAME_SPEED 30 		/*Number of steps the game takes per second*/
#define PRECISION 10		/*coordinate values within a col or row*/
#define MISSILE_DELAY 10	/*number of steps between missile launches*/
#define MISSILE_REWARD 3	/*number missles earned from a hit*/
#define MAX_ESCAPE 10		/*number of escaped allowed before game over*/
#define STATE_LIVE 1
#define STATE_DEAD 0
#define STATE_INIT 3
#define STATE_HIT 2

struct launcher_data {
	int x;
	int missiles_left;
};

struct missile_data {
	int x;
	int y;
	int state;
};

struct saucer_data {
	int x;
	int y;
	int state;
	int speed;
};

static void killhandler();

static void * launcher_init(void *);
static void * saucer_init(void *);
static void * missile_init(void *);

#endif

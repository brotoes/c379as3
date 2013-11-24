#ifndef SAUCER_H
#define SAUCER_H

#define MAX_SAUCERS 20
#define MAX_MISSILES 30
#define SAUCER_REGION_TOP 0
#define SAUCER_REGION_BOT 5
#define MISSILE_SPEED 5
#define SAUCER_SPEED_TOP 4
#define SAUCER_SPEED_LOW 1
#define START_MISSILES 10
#define START_SAUCER_RATE 5
#define GAME_SPEED 30

struct missile_data {
	int x;
	int y;
	int ttl;
};

struct saucer_data {
	int x;
	int y;
	int ttl;
	int speed;
};

void * launcher_init(void *);
void * saucer_init(void *);
void * missile_init(void *);

#endif

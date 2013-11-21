#ifndef SAUCER_H
#define SAUCER_H

#define SAUCER_REGION_TOP 0
#define SAUCER_REGION_BOT 5
#define MISSILE_SPEED 5
#define SAUCER_SPEED_TOP 4
#define SAUCER_SPEED_LOW 1

struct launcher_data

struct missile_data {
	int x;
	int y;
}

struct saucer_data {
	int x;
	int y;
	int dead;
	int speed;
}

void * launcher_init(void *);
void * saucer_init(void *);
void * missile_init(void *);

#endif

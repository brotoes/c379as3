#ifndef SAUCER_H
#define SAUCER_H

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

struct drawer_data{
    char * cells;
    int escaped;
    int rockets;
    int score;
}

void * launcher(void *);
void * saucer(void *);
void * missile(void *);
void * drawer(void *);

#endif

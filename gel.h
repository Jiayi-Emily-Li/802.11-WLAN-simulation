#ifndef GEL_H
#define GEL_H

struct event {
    int srcHost;
    int destHost;
	double timeS;
    double timeL;
    int type;
    double backoff;
    double delay;
    int attempts;
};

typedef struct dll* dll_t;
typedef struct event* event_t;

dll_t gel_create();
event_t create_event(int srcHost, int destHost, double timeS, double timeL, int type , double backoff, double delay, int attempts);
int gel_insert(dll_t gel, event_t event);
int gel_remove(dll_t gel, event_t* event);
int gel_destroy(dll_t gel);
int gel_length(dll_t gel);

#endif
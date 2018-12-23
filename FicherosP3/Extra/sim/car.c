#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "barrier.h"
#include "bridge.h"
#include "car.h"

pthread_mutex_t mtx_arrival;
pthread_cond_t vc_arrival;

pthread_mutex_t mtx_time;

extern FILE *outfile;
extern int local_time;
extern sys_barrier_t mybarrier;

int hilos=0;

extern int num_threads;


void init_car(tcar *car)
{
	sys_barrier_wait(&mybarrier);
	sys_barrier_wait(&mybarrier);

	pthread_mutex_lock(&mtx_arrival);

	while(local_time<car->t_arrival)
		pthread_cond_wait(&vc_arrival, &mtx_arrival);

	pthread_mutex_unlock(&mtx_arrival);
}

void stat_car_in(tcar *car, int ncars_on_the_bridge)
{
	sleep(1);
	pthread_cond_broadcast(&vc_arrival);
	pthread_mutex_lock(&mtx_time);
	if (car->type=='b' && ncars_on_the_bridge>1 && ncars_on_the_bridge!=3)
		local_time++;
	car->t_cross_in = local_time;
	printf("local_time(%d)=%d\n", car->id, local_time);
	local_time++;
	pthread_mutex_unlock(&mtx_time);
	pthread_cond_broadcast(&vc_arrival);
}

void stat_car_out(tcar *car)
{

	if (dbridge.cars_on_bridge == 0 
		&& !dbridge.cars_waiting[car->my_direction])
	{
		pthread_mutex_lock(&mtx_time);
		local_time++; local_time+=LENGTH-2;
printf("local_time_extra(%d)=%d\n", car->id, local_time);
		pthread_mutex_unlock(&mtx_time);
	}
}

void write_stats(tcar *car)
{
	pthread_mutex_lock(&mtx_arrival);
	car->t_cross_out = car->t_cross_in+LENGTH;

	fprintf(outfile,"%c%d\t%d\t%d\tready\n", car->type, car->id, car->t_arrival, car->t_cross_in);
	if (car->my_direction)
		fprintf(outfile,"%c%d\t%d\t%d\tWE\n", car->type, car->id, car->t_cross_in, car->t_cross_out);
	else
		fprintf(outfile,"%c%d\t%d\t%d\tEW\n", car->type, car->id, car->t_cross_in, car->t_cross_out);

	pthread_mutex_unlock(&mtx_arrival);
}

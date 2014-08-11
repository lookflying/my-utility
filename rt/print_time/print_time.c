#include <stdio.h>
#include <time.h>

unsigned long long ts_to_ns_ull(struct timespec *t)
{
	return (unsigned long long)t->tv_sec * 1E9 + (unsigned long long)t->tv_nsec ;
}

unsigned long long ts_to_us_ull(struct timespec *t)
{
	return (unsigned long long)t->tv_sec * 1E6 + (unsigned long long)t->tv_nsec/1000 ;
}


void print_cur_ts(void)
{
	struct timespec t;
	unsigned long long t_ull;
	clock_gettime(CLOCK_MONOTONIC, &t);
	printf("ts = %llu\n", ts_to_us_ull(&t));
}

int main(int argc, char* argv[])
{
	while(1)
	{
		print_cur_ts();
	}
}

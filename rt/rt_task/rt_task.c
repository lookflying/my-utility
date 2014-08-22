#include "dl_syscalls.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
static struct timespec t_deadline;
static volatile int running = 0;
static volatile int ready = 0;
static volatile int started = 0;
static struct timespec dl_period, dl_budget, dl_exec;
static int lack_cnt = 0, miss_cnt = 0;
static int cnt = 0;//task counter
static struct timespec t_exit;
static int duration = 0;
static int log = 1;
static struct timespec t_task_start, t_task_life;


static int64_t timespec_to_nsec(struct timespec *ts)
{
	return ts->tv_sec * 1E9 + ts->tv_nsec;
}


struct timespec usec_to_timespec(unsigned long usec)
{
	struct timespec ts;

	ts.tv_sec = usec / 1000000;
	ts.tv_nsec = (usec % 1000000) * 1000;
	
	return ts;
}

struct timespec msec_to_timespec(unsigned int msec)
{
	struct timespec ts;

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;

	return ts;
}

struct timespec timespec_add(struct timespec *t1, struct timespec *t2)
{
	struct timespec ts;

	ts.tv_sec = t1->tv_sec + t2->tv_sec;
	ts.tv_nsec = t1->tv_nsec + t2->tv_nsec;

	while (ts.tv_nsec >= 1E9) {
		ts.tv_nsec -= 1E9;
		ts.tv_sec++;
	}

	return ts;
}

struct timespec timespec_sub(struct timespec *t1, struct timespec *t2)
{
	struct timespec ts;
	
	if (t1->tv_nsec < t2->tv_nsec) {
		ts.tv_sec = t1->tv_sec - t2->tv_sec -1;
		ts.tv_nsec = t1->tv_nsec  + 1000000000 - t2->tv_nsec; 
	} else {
		ts.tv_sec = t1->tv_sec - t2->tv_sec;
		ts.tv_nsec = t1->tv_nsec - t2->tv_nsec; 
	}

	return ts;

}

int timespec_lower(struct timespec *what, struct timespec *than)
{
	if (what->tv_sec > than->tv_sec)
		return 0;

	if (what->tv_sec < than->tv_sec)
		return 1;

	if (what->tv_nsec < than->tv_nsec)
		return 1;

	return 0;
}

void print_result_exit(struct timespec *t_now)
{	
	struct timespec t_duration;
	int64_t i_duration;
	t_duration = timespec_sub(t_now, &t_exit);
	i_duration = timespec_to_nsec(&t_duration) + (int64_t)duration * 1E9;

	printf("===begin===\nstart= %lld ns\nend= %lld ns\nduration= %lld ns\nlack_cnt= %d\t%3.2f%%\ncnt= %d\ncorrect_cnt= %lld\nmiss_cnt= %d\t%3.2f%%\naverage_duration_per_cnt= %lld ns\n",
						timespec_to_nsec(&t_exit) - (int64_t)(duration * 1E9),
						timespec_to_nsec(t_now),
						i_duration,
						lack_cnt,
						(double)lack_cnt/(double)cnt * 100.0,
						cnt,
						(int64_t)(duration * 1E9) / timespec_to_nsec(&dl_period),
						miss_cnt,
						(double)miss_cnt/(double)cnt * 100.0,
						(int64_t)i_duration / cnt);

	exit(0);
	
}


void signal_handler(int signum)
{
		switch(signum)
		{
			case SIGALRM:
			{
				if (ready)
				{
					if (!started)
					{
						clock_gettime(CLOCK_REALTIME, &t_deadline);
						t_deadline = timespec_add(&t_deadline, &dl_period);
						started = 1;
					}
					running = 0;
				}
				break;
			}
			case SIGINT:
			{
				struct timespec t_now;
				clock_gettime(CLOCK_REALTIME, &t_now);
				print_result_exit(&t_now);
				break;
			}
			default:
			{
				break;
			}
		}
}

static inline struct timespec busy_wait(struct timespec *to)
{
	struct timespec t_step, t_remaining;
	while(1)
	{
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_step);
		if (!running || !timespec_lower(&t_step, to))
		{
			t_remaining = timespec_sub(to, &t_step);
			return t_remaining;
		}
	}
}
static inline void print_log(struct timespec *start, struct timespec *end, struct timespec *diff, struct timespec *remaining, struct timespec *deadline, struct timespec *offset, struct timespec *slack)
{
	int64_t i_start, i_end, i_diff, i_remaining, i_deadline, i_slack, i_offset;
	i_start = timespec_to_nsec(start);
	i_end = timespec_to_nsec(end);
	i_diff = timespec_to_nsec(diff);
	i_remaining = timespec_to_nsec(remaining);
	i_deadline = timespec_to_nsec(deadline);
	i_offset = timespec_to_nsec(offset);
	i_slack = timespec_to_nsec(slack);
	if (i_remaining > 0)
	{
		++lack_cnt;
	}
	if (i_slack < 0)
	{
		++miss_cnt;
	}
	if (log)
	{
		printf("%19lld\t%19lld\t%19lld\t%19lld\t%19lld\t%19lld\t%19lld\n",
							i_start,
							i_end,
							i_diff,
							i_remaining, 
							i_deadline,
							i_offset, 
							i_slack);
	}
}
void bye(void)
{
	struct timespec t_life;	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_life);
	t_task_life = timespec_sub(&t_life, &t_task_start);
	printf("task_lifetime= %lld ns\nlifetime= %lld ns\n===end===\n", 
			timespec_to_nsec(&t_task_life),
			timespec_to_nsec(&t_life));
}
int main(int argc, char* argv[])
{	
	pid_t pid;
	long period, budget, exec;
	char* token;
	struct sched_attr dl_attr;
	int ret;
	unsigned int flags = 0;
	struct sigaction sa;
	struct itimerval timer;

	ret = atexit(bye);
	if (ret != 0)
	{
		perror("atexit");
		exit(1);
	}
	if (argc >= 2)
	{

		pid = getpid();
		printf("%d\t===pid===\n", (int)pid);

		printf("lock pages in memory\n");
		ret = mlockall(MCL_CURRENT | MCL_FUTURE);
		if (ret < 0)
		{
			perror("mlockall");
			exit(1);
		}
	
		token = strtok(argv[1], ":");
		period = strtol(token, NULL, 10);
		token = strtok(NULL, ":");
		budget = strtol(token, NULL, 10);
		token = strtok(NULL, ":");
		exec = strtol(token, NULL, 10);
		printf("period = %ld us, budget = %ld us, exec = %ld us\n", period, budget, exec);
		
		if (argc >= 3)
		{
			duration = atoi(argv[2]);
			clock_gettime(CLOCK_REALTIME, &t_exit);
			t_exit.tv_sec = t_exit.tv_sec + duration;
		}

		if (argc >= 4)
		{
			log = atoi(argv[3]);
		}
		pid = 0;

		assert(period >= budget && budget >= exec);
		dl_period = usec_to_timespec(period);
		dl_budget = usec_to_timespec(budget);
		dl_exec = usec_to_timespec(exec);
		dl_attr.size = sizeof(dl_attr);
		dl_attr.sched_flags = 0;
		dl_attr.sched_policy = SCHED_DEADLINE;
		dl_attr.sched_priority = 0;
		dl_attr.sched_runtime = timespec_to_nsec(&dl_budget) + (timespec_to_nsec(&dl_budget) / 100) * 5;
		dl_attr.sched_deadline = timespec_to_nsec(&dl_period);
		dl_attr.sched_period = timespec_to_nsec(&dl_period);
	

		ret = sched_setattr(pid, &dl_attr, flags);
		if (ret != 0)
		{
			perror("sched_setattr");
			exit(1);
		}

		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &signal_handler;
		sigaction(SIGALRM, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);
		timer.it_value.tv_sec = dl_period.tv_sec;
		timer.it_value.tv_usec = dl_period.tv_nsec / 1000;
		timer.it_interval.tv_sec = dl_period.tv_sec;
		timer.it_interval.tv_usec = dl_period.tv_nsec / 1000;
		setitimer(ITIMER_REAL, &timer, NULL);

		struct timespec t_start, t_end, t_diff, t_slack, t_now, t_exec, t_remaining, t_sleep, t_offset;
		ready = 1;
		while(!started);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_task_start);	
		while(1)
		{
			clock_gettime(CLOCK_REALTIME, &t_start);
			clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_now);
			t_exec = timespec_add(&t_now, &dl_exec);
			running = 1;
			t_remaining = busy_wait(&t_exec);
			++cnt;
			clock_gettime(CLOCK_REALTIME, &t_end);

			t_diff = timespec_sub(&t_end, &t_start);
			t_slack = timespec_sub(&t_deadline, &t_end);
			t_offset = timespec_sub(&t_deadline, &t_start);
			print_log(&t_start, &t_end, &t_diff, &t_remaining, &t_deadline, &t_offset, &t_slack);
			t_deadline = timespec_add(&t_deadline, &dl_period);
			t_sleep = t_deadline;
			//assert(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t_sleep, NULL) != 0);
			//clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t_deadline, NULL);
			while(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t_sleep, NULL) == 0)
			{
				t_sleep = timespec_add(&t_sleep, &dl_period);
			}
			
			clock_gettime(CLOCK_REALTIME, &t_now);
			if (duration && timespec_lower(&t_exit, &t_now))
			{
				print_result_exit(&t_now);
								
			}
		}

	}
	return 0;
}

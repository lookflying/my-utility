#include "dl_syscalls.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

static struct timespec usec_to_timespec(unsigned long usec)
{
	struct timespec ts;

	ts.tv_sec = usec / 1000000;
	ts.tv_nsec = (usec % 1000000) * 1000;

	return ts;
}

static int64_t timespec_to_nsec(struct timespec *ts)
{
	return ts->tv_sec * 1E9 + ts->tv_nsec;
}
/*
 * SCHED_DEADLINE system call
 */
int sched_setattr(pid_t pid,
		const struct sched_attr *attr,
		unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
		struct sched_attr *attr,
		unsigned int size,
		unsigned int flags)
{
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

int main(int argc, char* argv[])
{
	pid_t pid;
	long period, exec;
	char* token;
	struct sched_attr dl_attr;
	struct timespec dl_period, dl_exec;
	int ret;
	unsigned int flags = 0;
	if (argc == 3)
	{
		pid = (pid_t)atoi(argv[1]);
		token = strtok(argv[2], ":");
		period = strtol(token, NULL, 10);
		token = strtok(NULL, ":");
		exec = strtol(token, NULL, 10);
		printf("pid = %ld, period = %ld, exec = %ld\n", (long)pid, period, exec);

		dl_period = usec_to_timespec(period);
		dl_exec = usec_to_timespec(exec);
		dl_attr.size = sizeof(dl_attr);
		dl_attr.sched_flags = 0;
		dl_attr.sched_policy = SCHED_DEADLINE;
		dl_attr.sched_priority = 0;
		dl_attr.sched_runtime = timespec_to_nsec(&dl_exec);// + (timespec_to_nsec(&dl_exec) / 100) * 5;
		dl_attr.sched_deadline = timespec_to_nsec(&dl_period);
		dl_attr.sched_period = timespec_to_nsec(&dl_period);
		
		
		ret = sched_setattr(pid, &dl_attr, flags);
		if (ret != 0)
		{
			perror("sched_setattr");
			exit(1);
		}
	}
	else if (argc == 2)
	{
		pid = (pid_t)atoi(argv[1]);
		ret = sched_getattr(pid, &dl_attr, sizeof(dl_attr), 0);
		if (ret != 0)
		{
			perror("sched_getattr");
			exit(1);
		}
		printf("deadline=%lld\tperiod=%lld\texec=%lld\n", 
				dl_attr.sched_deadline,
				dl_attr.sched_period,
				dl_attr.sched_runtime);
	}
	else
	{
		printf("usage: set_deadline <pid> <period>:<exec>\n");
		printf("usage: set_deadline <pid>\n");
	}
	return 0;
}

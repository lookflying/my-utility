#include <stdio.h>

int main(int argc, char* argv[])
{
	printf("size of unsigned long %lu\n", sizeof(unsigned long));
	printf("largest unsigned long = %lu\n", (unsigned long)((long)0 - 1));
	printf("size of unsigned long long %lu\n", sizeof(unsigned long long));
	printf("largest unsigned long long  = %llu\n", (unsigned long long)((long long)0 - 1));
	return 0;
}

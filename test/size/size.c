#include <stdio.h>
int array[10];
int main(int argc, char* argv[])
{
	printf("size of unsigned long %lu\n", sizeof(unsigned long));
	printf("largest unsigned long = %lu\n", (unsigned long)((long)0 - 1));
	printf("size of unsigned long long %lu\n", sizeof(unsigned long long));
	printf("largest unsigned long long  = %llu\n", (unsigned long long)((long long)0 - 1));
	printf("sizeof(int array[10]) = %lu\n", sizeof(array));
	return 0;
}

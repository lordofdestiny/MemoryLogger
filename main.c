#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "memory_logger.h"

int main()
{
	FILE* output = fopen("text.txt", "w");
	if (output == NULL) {
		printf("Failed to open file!\n");
		exit(0);
	}
	memoryLoggerInit(output);
	memoryLoggerAtExitHook(1);

	int* x = malloc(sizeof(int), "x - int");
	double* y = calloc(10, sizeof(double), "y - double[10]");
	int* tempInt = realloc(x, 10 * sizeof(int));
	if (tempInt == NULL) {
		printf("Reallocation failed!\n");
		exit(0);
	}
	else {
		x = tempInt;
	}
	double* tempDouble = realloc(y, 5 * sizeof(int));
	if (tempDouble == NULL) {
		printf("Reallocation failed!\n");
		exit(0);
	}
	else {
		y = tempDouble;
	}

	printMemoryLogs();

	free(x);
	free(y);

	fclose(output);
}
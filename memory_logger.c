#include "memory_logger.h"

typedef struct {
	void* pointer;
	size_t bytes;
	char* name;
} Allocation;

static struct {
	size_t allocated, freed;
	size_t allocationCount, deallocationCount, reallocationCount;
	Allocation* pointers;
} memoryFootprint;

static FILE* outFile;
static int initialized = 0;
static int printAtExit = 0;

static FILE* getOutFile() {
	return outFile != NULL ? outFile : stdout;
}

static void atExitHook() {
	if (printAtExit) {
		outFile = stdout;
		printMemoryLogs();
	}

	(free)(memoryFootprint.pointers);
}

void memoryLoggerInit(FILE* out) {
	if (initialized) {
		return;
	}
	initialized = 1;
	outFile = out != NULL ? out : stdout;
	atexit(atExitHook);

	memoryFootprint.pointers = (calloc)(MAX_POINTER_COUNT, sizeof(Allocation));
}

int memoryLoggerAtExitHook(int status) {
	return printAtExit = status != 0;
}

void* m_malloc(size_t size, char* name) {
	void* temp = (malloc)(size);
	Allocation* helper = &memoryFootprint.pointers[memoryFootprint.allocationCount];
	helper->pointer = temp;
	helper->bytes = size;
	helper->name = name;
	memoryFootprint.allocated += size;
	memoryFootprint.allocationCount += 1;
	fprintf(getOutFile(), "%zu bytes allocated for \"%s\"!\n", size, name);
	fprintf(getOutFile(), "%zu bytes still allocated!\n",
		memoryFootprint.allocated - memoryFootprint.freed);
	fprintf(getOutFile(), "--------------------------------------------\n");
	return temp;
}

void* m_calloc(size_t count, size_t size, char* name) {
	void* temp = (calloc)(count, size);
	Allocation* helper = &memoryFootprint.pointers[memoryFootprint.allocationCount];
	helper->pointer = temp;
	helper->bytes = count * size;
	helper->name = name;
	memoryFootprint.allocated += count * size;
	memoryFootprint.allocationCount += 1;
	fprintf(getOutFile(), "%zu bytes allocated for \"%s\"!\n", count * size, name);
	fprintf(getOutFile(), "%zu bytes still allocated!\n",
		memoryFootprint.allocated - memoryFootprint.freed);
	fprintf(getOutFile(), "--------------------------------------------\n");
	return temp;
}

void* m_realloc(void* memory, size_t size) {
	void* temp = (realloc)(memory, size);
	for (size_t i = 0; i < memoryFootprint.allocationCount; i++) {
		Allocation* helper = &memoryFootprint.pointers[i];
		if (helper->pointer == NULL) continue;
		if (helper->pointer == memory) {
			if (temp == NULL) {
				fprintf(getOutFile(), "Reallocation of pointer %p with \"name\" %s failed!\n",
					helper->pointer, helper->name);
				fprintf(getOutFile(), "--------------------------------------------\n");
				break;
			}
			helper->pointer = temp;
			if (size >= helper->bytes) {
				size_t difference = size - helper->bytes;
				memoryFootprint.allocated += difference;
				helper->bytes += difference;
				fprintf(getOutFile(), "Additional %zu bytes allocated for %s!\n",
					difference, helper->name);
			}
			else {
				size_t difference = helper->bytes - size;
				memoryFootprint.freed += difference;
				helper->bytes -= difference;
				fprintf(getOutFile(), "%zd bytes freed from %s!\n",
					difference, helper->name);
			}
			fprintf(getOutFile(), "%zu bytes still allocated!\n",
				memoryFootprint.allocated - memoryFootprint.freed);
			memoryFootprint.reallocationCount += 1;
			fprintf(getOutFile(), "--------------------------------------------\n");
			break;
		}
	}
	return temp;
}

void m_free(void* memory) {
	for (size_t i = 0; i < memoryFootprint.allocationCount; i++) {
		Allocation* helper = &memoryFootprint.pointers[i];
		if (helper->pointer == NULL) continue;
		if (helper->pointer == memory) {
			memoryFootprint.freed += memoryFootprint.pointers[i].bytes;
			fprintf(getOutFile(), "%zu bytes freed for \"%s\"!\n",
				helper->bytes, helper->name);
			fprintf(getOutFile(), "%zu bytes still allocated!\n",
				memoryFootprint.allocated - memoryFootprint.freed);
			memoryFootprint.deallocationCount += 1;
			fprintf(getOutFile(), "--------------------------------------------\n");
			helper->pointer = NULL;
			helper->bytes = 0;
			break;
		}
	}
	(free)(memory);
}

void (printMemoryLogs)() {
	if (memoryFootprint.allocated == memoryFootprint.freed) {
		fprintf(getOutFile(), "All allocated memory locations were properly cleaned!");
		return;
	}
	fprintf(getOutFile(), "***********************************************\n");
	fprintf(getOutFile(), "Number of allocations: %zu\n",
		memoryFootprint.allocationCount);
	fprintf(getOutFile(), "Number of deallocations: %zu\n",
		memoryFootprint.deallocationCount);
	fprintf(getOutFile(), "Number of reallocations: %zu\n",
		memoryFootprint.reallocationCount);
	fprintf(getOutFile(), "---------------------------------------------\n");
	for (size_t i = 0; i < memoryFootprint.allocationCount; i++) {
		if (memoryFootprint.pointers[i].pointer != NULL) {
			fprintf(getOutFile(), "Non-deallocated memory at pointer 0x%p.\n",
				memoryFootprint.pointers[i].pointer);
			fprintf(getOutFile(), "Size = %zu; Name = \"%s\"\n",
				memoryFootprint.pointers[i].bytes,
				memoryFootprint.pointers[i].name);
			fprintf(getOutFile(), i + 1 != memoryFootprint.allocationCount ?
				"---------------------------------------------\n" : "");
		}
	}
	fprintf(getOutFile(), "***********************************************\n");
}

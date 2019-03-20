#include <stddef.h> /* size_t */
#include <assert.h> /* assert */
#include "Time.h"

struct Time {
	int key;
	char rdm[1000];
};

#define POOL_NAME Time
#define POOL_TYPE struct Time
#include "StablePool.h"

#define ARRAY_NAME Time
#define ARRAY_TYPE struct Time
#include "Array.h"

#define ARRAY_NAME FreeTime
#define ARRAY_TYPE struct Time
#define ARRAY_FREE_LIST
#include "Array.h"

void TestPool(const size_t replicas) {
	size_t i;
	struct TimePool a;
	for(i = 0; i < replicas; i++) {
		TimePool(&a);
		TimePool_(&a);
	}
}

void TestArray(const size_t replicas) {
	size_t i;
	struct TimeArray a;
	for(i = 0; i < replicas; i++) {
		TimeArray(&a);
		TimeArray_(&a);
	}
}

void TestFreeArray(const size_t replicas) {
	size_t i;
	struct FreeTimeArray a;
	for(i = 0; i < replicas; i++) {
		FreeTimeArray(&a);
		FreeTimeArray_(&a);
	}
}

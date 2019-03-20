#include <stdio.h> /* snprintf */
#include <stddef.h> /* size_t */
#include <assert.h> /* assert */
#include "Orcish.h"
#include "Time.h"

struct Time {
	int key;
	char rdm[300];
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

static struct Time s_time[10000];
static const size_t s_time_no = sizeof s_time / sizeof *s_time;
static size_t s_time_count;
static struct Time *a_time[10000];
static const size_t a_time_no = sizeof a_time / sizeof *a_time;
static size_t a_time_count;

static void print_time(const struct Time *const t, char (*const a)[12]) {
	assert(t && a);
	snprintf(*a, sizeof *a, "%s:%d", t->rdm, t->key);
}

static void init_time(struct Time *const t) {
	assert(t);
	t->key = rand();
	Orcish(t->rdm, sizeof t->rdm);
}

static void print_alloc(void) {
	size_t i;
	char a[12];
	printf("{");
	for(i = 0; i < a_time_count; i++) {
		print_time(a_time[i], &a);
		printf("%s%s", i ? ", " : "", a);
	}
	printf("}\n");
}

void TestStatic(const size_t replicas) {
	size_t i;
	char a[12];
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *t;
		if(r < 1.0 * s_time_count / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * s_time_count;
			printf("des %lu\n", (unsigned long)idx);
			if(idx < --s_time_count)
				memcpy(s_time + idx, s_time + s_time_count, sizeof *t);
		} else {
			if(s_time_count >= s_time_no)
				{ fprintf(stderr, "TestStatic too big."); continue; }
			t = s_time + s_time_count++;
			init_time(t);
			print_time(t, &a);
			printf("create %s\n", a);
		}
	}
	s_time_count = 0;
}

void TestAlloc(const size_t replicas) {
	size_t i;
	char a[12];
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *t;
		if(r < 1.0 * a_time_count / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * a_time_count;
			assert(idx < a_time_count);
			printf("des %lu\n", (unsigned long)idx);
			t = a_time[idx], assert(t);
			free(t);
			if(idx < --a_time_count)
				a_time[idx] = a_time[a_time_count];
		} else {
			if(a_time_count >= a_time_no)
				{ fprintf(stderr, "TestAlloc too big."); continue; }
			if(!(t = malloc(sizeof *t)))
				{ perror("malloc"); continue; }
			a_time[a_time_count++] = t;
			init_time(t);
			print_time(t, &a);
			printf("create %s\n", a);
			print_alloc();
		}
	}
	while(a_time_count) free(a_time[--a_time_count]);
}

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

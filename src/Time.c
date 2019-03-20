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

#define ARRAY_NAME ConTime
#define ARRAY_TYPE struct Time
#define ARRAY_TAIL_REMOVED
#include "Array.h"

#define ARRAY_NAME FreeTime
#define ARRAY_TYPE struct Time
#define ARRAY_FREE_LIST
#include "Array.h"

static struct Time s_time[500000];
static const size_t s_time_no = sizeof s_time / sizeof *s_time;
static size_t s_time_count;
static struct Time *a_time[500000];
static const size_t a_time_no = sizeof a_time / sizeof *a_time;
static size_t a_time_count;

static void init_time(struct Time *const t) {
	assert(t);
	t->key = rand();
	Orcish(t->rdm, sizeof t->rdm);
}

#ifdef PRINT
static void time_to_string(const struct Time *const t, char (*const a)[12]) {
	assert(t && a);
	snprintf(*a, sizeof *a, "%s:%d", t->rdm, t->key);
}
static void print_alloc(void) {
	size_t i;
	char a[12];
	printf("{");
	for(i = 0; i < a_time_count; i++) {
		time_to_string(a_time[i], &a);
		printf("%s%s", i ? ", " : "", a);
	}
	printf("}\n");
}
#endif

#define MODIFY

static void modify(struct Time *const t) {
	assert(t);
#ifdef MODIFY
	t->key++;
#elif defined(MODIFY_GREATLY)
	init_time(t);
#endif
}

void TestStatic(const size_t replicas) {
	size_t i, j;
#ifdef PRINT
	char a[12];
#endif
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *time;
		if(r < 1.0 * s_time_count / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * s_time_count;
#ifdef PRINT
			printf("des %lu\n", (unsigned long)idx);
#endif
			if(idx < --s_time_count)
				memcpy(s_time + idx, s_time + s_time_count, sizeof *time);
		} else {
			if(s_time_count >= s_time_no)
				{ fprintf(stderr, "TestStatic too big."); return; }
			time = s_time + s_time_count++;
			init_time(time);
#ifdef PRINT
			init_time(t);
			time_to_string(t, &a);
			printf("create %s\n", a);
#endif
		}
		for(j = 0; j < s_time_count; j++) modify(s_time + j);
	}
	s_time_count = 0;
}

void TestAlloc(const size_t replicas) {
	size_t i, j;
#ifdef PRINT
	char a[12];
#endif
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *time;
		if(r < 1.0 * a_time_count / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * a_time_count;
			assert(idx < a_time_count);
#ifdef PRINT
			printf("des %lu\n", (unsigned long)idx);
#endif
			time = a_time[idx], assert(time);
			free(time);
			if(idx < --a_time_count)
				a_time[idx] = a_time[a_time_count];
		} else {
			if(a_time_count >= a_time_no)
				{ fprintf(stderr, "TestAlloc too big."); return; }
			if(!(time = malloc(sizeof *time)))
				{ perror("malloc"); continue; }
			a_time[a_time_count++] = time;
			init_time(time);
#ifdef PRINT
			time_to_string(t, &a);
			printf("create %s\n", a);
			print_alloc();
#endif
		}
		for(j = 0; j < a_time_count; j++) modify(a_time[j]);
	}
	while(a_time_count) free(a_time[--a_time_count]);
}

void TestPool(const size_t replicas) {
	size_t i, pool_size = 0;
#ifdef PRINT
	char a[12];
#endif
	struct TimePool t;
	TimePool(&t);
	/*TimePoolReserve(&t, 3000);*/
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *time;
		if(r < 1.0 * pool_size / replicas) {
			/*struct pool_Time_Block *b;
			struct pool_Time_Node *n;*/
			size_t idx = rand() / (1.0 + RAND_MAX) * pool_size;
			assert(idx < pool_size && t.largest && pool_size);
#ifdef PRINT
			printf("des %lu\n", (unsigned long)idx);
#endif
			/*
			for(b = t.largest->smaller; b; b = b->smaller)
				{ if(idx < b->size) break; idx -= b->size; }
			if(b) {
				n = pool_Time_block_nodes(b) + idx;
			} else {
				n = pool_Time_block_nodes(t.largest) + idx;
				while(n->x.prev) n++;
			}
			*/
			TimePoolRemove(&t, a_time[--pool_size]);
		} else {
			if(!(time = TimePoolNew(&t))) { perror("TimeArray"); continue; }
			a_time[pool_size++] = time;
			init_time(time);
#ifdef PRINT
			time_to_string(time, &a);
			printf("create %s\n", a);
			print_alloc();
#endif
		}
		TimePoolForEach(&t, &modify);
	}
	TimePool_(&t);
}

void TestArray(const size_t replicas) {
	size_t i;
	struct TimeArray t;
#ifdef PRINT
	char a[12];
#endif
	TimeArray(&t);
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *time;
		if(r < 1.0 * TimeArraySize(&t) / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * TimeArraySize(&t);
#ifdef PRINT
			printf("des %lu\n", (unsigned long)idx);
#endif
			time = TimeArrayGet(&t, idx), assert(time);
			if(!TimeArrayRemove(&t, time)) perror("TimeArray");
		} else {
			if(!(time = TimeArrayNew(&t))) { perror("TimeArray"); continue; }
			init_time(time);
#ifdef PRINT
			time_to_string(time, &a);
			printf("create %s\n", a);
			print_alloc();
#endif
		}
		TimeArrayForEach(&t, &modify);
	}
	TimeArray(&t);
}

void TestConArray(const size_t replicas) {
	size_t i;
	struct ConTimeArray t;
#ifdef PRINT
	char a[12];
#endif
	ConTimeArray(&t);
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *time;
		if(r < 1.0 * ConTimeArraySize(&t) / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * ConTimeArraySize(&t);
#ifdef PRINT
			printf("des %lu\n", (unsigned long)idx);
#endif
			time = ConTimeArrayGet(&t, idx), assert(time);
			if(!ConTimeArrayRemove(&t, time)) perror("TimeArray");
		} else {
			if(!(time = ConTimeArrayNew(&t))) { perror("TimeArray"); continue; }
			init_time(time);
#ifdef PRINT
			time_to_string(time, &a);
			printf("create %s\n", a);
			print_alloc();
#endif
		}
		ConTimeArrayForEach(&t, &modify);
	}
	ConTimeArray(&t);
}

void TestFreeArray(const size_t replicas) {
	size_t i, pool_size = 0;
	struct FreeTimeArray t;
#ifdef PRINT
	char a[12];
#endif
	FreeTimeArray(&t);
	for(i = 0; i < replicas; i++) {
		float r = (float)(rand() / (1.0 + RAND_MAX));
		struct Time *time;
		if(r < 1.0 * pool_size / replicas) {
			size_t idx = rand() / (1.0 + RAND_MAX) * pool_size;
			assert(idx < pool_size);
#ifdef PRINT
			printf("des %lu\n", (unsigned long)idx);
#endif
			while(!(time = FreeTimeArrayGet(&t, idx))) idx++;
			FreeTimeArrayRemove(&t, time);
			pool_size--;
		} else {
			if(!(time = FreeTimeArrayNew(&t))) { perror("TimeArray"); continue; }
			pool_size++;
			init_time(time);
#ifdef PRINT
			time_to_string(time, &a);
			printf("create %s\n", a);
			print_alloc();
#endif
		}
		FreeTimeArrayForEach(&t, &modify);
	}
	FreeTimeArray_(&t);
}

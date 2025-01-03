/* Intended to be included on `POOL_TEST`. */

#include <limits.h>
#include <stdlib.h>

#	if defined QUOTE || defined QUOTE_
#		error Cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)
#	ifdef static /* Private functions. */
#		undef static
#	endif

/** Operates by side-effects. */
typedef void (*pT_(action_fn))(pT_(type) *);

/** Crashes if `pool` is not in a valid state. */
static void pT_(valid_state)(const struct t_(pool) *const pool) {
	size_t i;
	if(!pool) return;
	/* If there's no capacity, there's no slots. */
	if(!pool->capacity0) assert(!pool->slots.size);
	/* Every slot up to size is active. */
	for(i = 0; i < pool->slots.size; i++) {
		assert(pool->slots.data[i].slab);
		assert(!i || pool->slots.data[i].size);
	}
	if(!pool->slots.size) {
		/* There are no free0 without slots. */
		assert(!pool->free0.as_array.size);
	} else {
		/* size[0] <= capacity0 */
		assert(pool->slots.data[0].size <= pool->capacity0);
		/* The free-heap indices are strictly less than the size. */
		for(i = 0; i < pool->free0.as_array.size; i++)
			assert(pool->free0.as_array.data[i] < pool->slots.data[0].size);
	}
}

static void pT_(test_states)(void) {
	struct t_(pool) pool = t_(pool)();
	pT_(type) *t, *slab;
	const size_t size[] = { 9, 14, 22 };
	enum { CHUNK1_IS_ZERO, CHUNK2_IS_ZERO } conf = CHUNK1_IS_ZERO;
	size_t i;
	int r;
	char z[12];

	printf("Test null.\n");
	errno = 0;
	pT_(valid_state)(0);

	printf("Empty.\n");
	pT_(valid_state)(&pool);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-01-idle.gv");

	/** `<P>filler` must be a function implementing <typedef:<PP>action_fn>. */
	printf("One element.\n");
	t = T_(new)(&pool), assert(t), t_(filler)(t), pT_(valid_state)(&pool);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-02-one.gv");

	printf("Remove.\n");
	r = T_(remove)(&pool, pool.slots.data[0].slab + 0), assert(r),
		pT_(valid_state)(&pool);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-03-remove.gv");

	printf("Pool buffer %lu.\n", (unsigned long)size[0]);
	r = T_(buffer)(&pool, size[0]), assert(r), pT_(valid_state)(&pool);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-04-buffer.gv");

	for(i = 0; i < size[0]; i++) t = T_(new)(&pool), assert(t),
		t_(filler)(t), pT_(valid_state)(&pool);
	assert(pool.slots.size == 1);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-05-one-slab.gv");
	for(i = 0; i < size[1]; i++) t = T_(new)(&pool), assert(t),
		t_(filler)(t), pT_(valid_state)(&pool);
	assert(pool.slots.size == 2);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-06-two-slabs.gv");
	t = T_(new)(&pool), assert(t), t_(filler)(t), pT_(valid_state)(&pool);
	assert(pool.slots.size == 3);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-07-three-slabs.gv");
	/* It's `malloc` whether, `conf = { 0: { 2, 0, 1 }, 1: { 2, 1, 0 } }`. */
	if(pool.slots.data[1].size == size[0]) conf = CHUNK1_IS_ZERO;
	else assert(pool.slots.data[1].size == size[1]);
	if(pool.slots.data[2].size == size[0]) conf = CHUNK2_IS_ZERO;
	else assert(pool.slots.data[2].size == size[1]);
	assert(pool.slots.data[0].size == 1);
	printf("%s is %u, %s is %u.\n", orcify(pool.slots.data[1].slab), conf,
		orcify(pool.slots.data[2].slab), !conf);
	t = pool.slots.data[!conf + 1].slab + 0, t_(to_string)(t, &z);
	printf("Removing index-zero %s from slab %u %s.\n", z, !conf + 1,
		orcify(pool.slots.data[!conf + 1].slab));
	T_(remove)(&pool, t), pT_(valid_state)(&pool);
	t = pool.slots.data[0].slab + 0, t_(to_string)(t, &z);
	printf("Removing index-zero %s from slab %u %s.\n", z, 0,
		orcify(pool.slots.data[0].slab));
	T_(remove)(&pool, pool.slots.data[0].slab + 0);
	pT_(valid_state)(&pool);
	assert(pool.slots.data[conf + 1].size == size[0]);
	slab = pool.slots.data[conf + 1].slab;
	printf("Removing all in slab %s.\n", orcify(slab));
	for(i = 0; i < size[0]; i++) T_(remove)(&pool, slab + i);
	pT_(valid_state)(&pool);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-08-remove-slab.gv");
	assert(pool.slots.size == 2);
	T_(clear)(&pool);
	assert(pool.slots.size == 1);
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-09-clear.gv");

	/* Remove at random. */
	for(i = 0; i < size[2]; i++) t = T_(new)(&pool), assert(t), t_(filler)(t), pT_(valid_state)(&pool);
	{
		const size_t n1 = size[2] - 1, n0 = n1 >> 1;
		size_t n;
		unsigned bits = 0, copy, copy_bit, rnd, rnd_bit;
		srand((unsigned)clock());
		assert(n1 <= sizeof bits * CHAR_BIT);
		for(n = 0; n < n0; n++) {
			rnd = (unsigned)rand() / (RAND_MAX / (n1 - n) + 1);
			rnd_bit = 1 << rnd;
			copy = bits;
			while(copy) {
				copy_bit = copy;
				copy &= copy - 1;
				copy_bit ^= copy;
				if(copy_bit > rnd_bit) break;
				rnd++, rnd_bit <<= 1;
			}
			bits |= rnd_bit;
		}
		for(i = 0; i < size[2]; i++) if(bits & (1 << i))
			T_(remove)(&pool, pool.slots.data[0].slab + i);
		i = n0;
	}
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-10-remove.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0].size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.as_array.size == i);

	/* Add at random to an already removed. */
	while(i) t = T_(new)(&pool), assert(t),
		t_(filler)(t), pT_(valid_state)(&pool), i--;
	pT_(graph)(&pool, "graph/pool/" QUOTE(POOL_NAME) "-11-replace.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0].size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.as_array.size == 0);

	printf("Destructor:\n");
	t_(pool_)(&pool);
	pT_(valid_state)(&pool);
	printf("Done basic tests.\n\n");
}

static void pT_(test_random)(void) {
	struct t_(pool) pool = t_(pool)();
	/* This parameter controls how big the pool wants to be. */
	struct { size_t size; pT_(type) *data[2*5000/*10*/]; } test;
	const size_t test_size = sizeof test.data / sizeof *test.data,
		length = 120000; /* Controls how many iterations. */
	size_t i;
	char graph_fn[64];
	const size_t graph_max = 100000;
	pT_(type) *data, **ptr;

	test.size = 0;
	for(i = 0; i < length; i++) {
		char str[12];
		unsigned r = (unsigned)rand();
		int is_print = !(i & (i - 1));
		if(r > test.size * (RAND_MAX / test_size) && test.size < test_size) {
			/* Create. */
			data = T_(new)(&pool);
			if(!data) { perror("Error"), assert(0); return;}
			t_(filler)(data);
			t_(to_string)(data, &str);
			if(is_print) printf("%lu: Created %s.\n", (unsigned long)i, str);
			test.data[test.size++] = data;
		} else {
			/* Remove. */
			int ret;
			assert(test.size);
			ptr = test.data + (unsigned)rand() / (RAND_MAX / test.size + 1);
			t_(to_string)(*ptr, &str);
			if(is_print) printf("%lu: Removing %s.\n", (unsigned long)i, str);
			ret = T_(remove)(&pool, *ptr);
			assert(ret || (perror("Removing"),
				pT_(graph)(&pool,
				"graph/pool/" QUOTE(POOL_NAME) "-rem-err.gv"), 0));
			/* test.remove(ptr) */
			memmove(ptr, ptr + 1,
				sizeof *test.data * (--test.size - (size_t)(ptr - test.data)));
		}
		if(is_print) printf("test.size: %lu.\n", (unsigned long)test.size);
		if(is_print && i < graph_max) {
			sprintf(graph_fn, "graph/pool/" QUOTE(POOL_NAME) "-step-%lu.gv",
				(unsigned long)(i + 1));
			pT_(graph)(&pool, graph_fn);
			printf("%s.\n", T_(to_string)(&pool));
		}
		pT_(valid_state)(&pool);
	}
	if(i < graph_max) {
		sprintf(graph_fn, "graph/pool/" QUOTE(POOL_NAME) "-step-%lu-end.gv",
			(unsigned long)i);
		pT_(graph)(&pool, graph_fn);
	}
	t_(pool_)(&pool);
}

#	define BOX_PUBLIC_OVERRIDE
#	include "../src/box.h"

/** The list will be tested on stdout; requires `POOL_TEST` and not `NDEBUG`.
 @allow */
static void T_(test)(void) {
	printf("<" QUOTE(POOL_NAME) ">pool: of type <" QUOTE(POOL_TYPE)
		"> was created using: POOL_TO_STRING; POOL_TEST; testing:\n");
	pT_(test_states)();
	pT_(test_random)();
	fprintf(stderr, "Done tests of <" QUOTE(POOL_NAME) ">pool.\n\n");
}

#	define BOX_PRIVATE_AGAIN
#	include "../src/box.h"

#	undef QUOTE
#	undef QUOTE_

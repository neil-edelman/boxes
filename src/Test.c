#include <stdlib.h>	/* EXIT_SUCCESS */
#include <stdio.h>	/* fprintf */
#include <time.h>	/* clock */
#include <math.h>	/* pow */

/* constants */
static const char *const gnu_name     = "sort.p";
static const char *const graph_name   = "sort.eps";
static const double impatient_ms      = 500.0;
static const double sample_grow_power = 4.0;
static const unsigned replicas        = 5;

/* a data structure with two doubly-linked lists */
struct Foo {
	int x, y;
	struct Foo *x_prev, *x_next;
	struct Foo *y_prev, *y_next;
};

/* the first elements of one list */
struct FooFirsts {
	struct Foo *x_first, *y_first;
};

/* for qsort_keys */
struct Key {
	int key;
	struct Foo *reference;
};

/* function type used in TimeData */
typedef void (*SortFn)(struct FooFirsts *const);

/* prototypes */
static int Foo_compare_x(const struct Foo *, const struct Foo *);
static int Foo_compare_y(const struct Foo *, const struct Foo *);

/* generate FooXLinkSort */
#define LINK_NAME FooX
#define LINK_TYPE struct Foo
#define LINK_PREV x_prev
#define LINK_NEXT x_next
#define LINK_COMPARATOR &Foo_compare_x
#include "LinkSort.h"

/* generate FooYLinkSort */
#define LINK_NAME FooY
#define LINK_TYPE struct Foo
#define LINK_PREV y_prev
#define LINK_NEXT y_next
#define LINK_COMPARATOR &Foo_compare_y
#include "LinkSort.h"

/* scratch space */
static struct Foo foo[1000000];
static const unsigned foo_capacity = sizeof foo / sizeof *foo;

/* random seed */
static unsigned seed;

/** @implements	FooXLinkCompare */
static int Foo_compare_x(const struct Foo *a, const struct Foo *b) {
	return a->x - b->x;
}

/** @implements	FooYLinkCompare */
static int Foo_compare_y(const struct Foo *a, const struct Foo *b) {
	return a->y - b->y;
}

/** @implements	int (*)(const void *, const void *) */
static int Key_compare(const void *a_key, const void *b_key) {
	const struct Key *a = (struct Key *)a_key, *b = (struct Key *)b_key;
	return a->key - b->key;
}

/*static void Foo_out(const struct FooFirsts *const this) {
	struct Foo *iter;
	int is_first;
	
	printf("x: ");
	for(iter = this->x_first, is_first = -1; iter; iter = iter->x_next) {
		printf("%s%d", is_first ? "[ " : ", ", iter->x);
		is_first = 0;
	}
	printf(" ]\ny: ");
	for(iter = this->y_first, is_first = -1; iter; iter = iter->y_next) {
		printf("%s%d", is_first ? "[ " : ", ", iter->y);
		is_first = 0;
	}
	printf(" ]\n");
}*/

/** @implements	SortFn */
static void concurrent(struct FooFirsts *const this) {
	#pragma omp parallel sections
	{
		#pragma omp section
		FooXLinkSort(&this->x_first);
		#pragma omp section
		FooYLinkSort(&this->y_first);
	}
}

/** @implements	SortFn */
static void serial(struct FooFirsts *const this) {
	FooXLinkSort(&this->x_first);
	FooYLinkSort(&this->y_first);
}

/** @implements	SortFn */
static void qsort_keys(struct FooFirsts *const this) {
	struct Foo *a;
	struct Key *keys = 0, *key;
	size_t keys_size = 0, keys_capacity[2] = { 13, 21 }, i;
	enum { E_NO_ERR, E_ERRNO } error = E_NO_ERR;

	/* "try-catch-finally" */
	do {
		/* initial malloc */
		if(!(keys = malloc(sizeof *keys * keys_capacity[0])))
			{ error = E_ERRNO; break; }
		/* copy all the keys into keys */
		for(a = this->x_first; a; a = a->x_next) {
			/* make sure we have space; fibonacci some more if need be */
			if(keys_size >= keys_capacity[0]) {
				struct Key *new_keys;
				keys_capacity[0] ^= keys_capacity[1];
				keys_capacity[1] ^= keys_capacity[0];
				keys_capacity[0] ^= keys_capacity[1];
				keys_capacity[1] += keys_capacity[0];
				if(keys_capacity[1] < keys_capacity[0])
					keys_capacity[1] = (size_t)-1;
				if(!(new_keys = realloc(keys, sizeof *keys * keys_capacity[0])))
					{ error = E_ERRNO; break; }
				keys = new_keys;
			}
			/* copy the element */
			key = keys + keys_size++;
			key->key = a->x;
			key->reference = a;
		}
		if(a) break; /* <- continued break from inner */
		/* qsort the keys */
		qsort(keys, keys_size, sizeof *keys, &Key_compare);
		/* map the order onto the linked-list */
		this->x_first = keys->reference;
		for(i = 0; i < keys_size; i++) {
			key = keys + i;
			key->reference->x_prev =              (i) ? keys[i-1].reference : 0;
			key->reference->x_next = (i!=keys_size-1) ? keys[i+1].reference : 0;
		}
		keys_size = 0;
		/* now sort y */
		for(a = this->y_first; a; a = a->y_next) {
			/* copy the element */
			key = keys + keys_size++;
			key->key = a->y;
			key->reference = a;
		}
		/* assert(keys_size == keys_size from before) */
		/* qsort the keys */
		qsort(keys, keys_size, sizeof *keys, &Key_compare);
		/* map the order onto the linked-list */
		this->y_first = keys->reference;
		for(i = 0; i < keys_size; i++) {
			key = keys + i;
			key->reference->y_prev =              (i) ? keys[i-1].reference : 0;
			key->reference->y_next = (i!=keys_size-1) ? keys[i+1].reference : 0;
		}
	} while(0);
	if(error == E_ERRNO) {
		perror("sort");
		free(keys);
		exit(EXIT_FAILURE);
	} {
		free(keys);
	}
}

#define NOISE

/** Sets up a random linked list of size elements and calls sort.
 @param size	Must be (0, foo_capacity).
 @return		The amount of time taken by sort. */
static double time_sort_ms(const SortFn sort, const unsigned size) {
	clock_t begin, end;
	unsigned i;
	struct Foo *a, *b;
	struct FooFirsts firsts;

	/* build up the linked list */
	firsts.x_first = firsts.y_first = foo;
	for(i = 0, a = foo; i < size; ++i, a++) {
		a->x_prev =            (!i) ? 0 : foo + i - 1;
		a->y_prev =            (!i) ? 0 : foo + i - 1;
		a->x_next = (i == size - 1) ? 0 : foo + i + 1;
		a->y_next = (i == size - 1) ? 0 : foo + i + 1;
#ifdef NOISE
		a->x = rand() / (RAND_MAX / 100.0f);
		a->y = rand() / (RAND_MAX / 100.0f);
#elif defined(NOISE_INCRESE)
		a->x = rand() / (RAND_MAX / 100.0f) + i;
		a->y = rand() / (RAND_MAX / 100.0f) + i;
#else
		a->x = size - i;
		a->y = size - i;
#endif
	}

	/* time */
	begin = clock();
	sort(&firsts);
	end   = clock();

	/* test that they're really in-order; assumes at least 1 datum */
	for(a = firsts.x_first, b = a->x_next, i = 1; b; a = b, b = b->x_next, i++) {
		if(a->x > b->x) {
			fprintf(stderr,
				"Not x-sorted at [ ... , %d, %d, ... ], seed %u; :0.\n",
				a->x, b->x, seed);
			exit(EXIT_FAILURE);
		}
	}
	if(i != size) {
		fprintf(stderr, "Input size %u, but output size %u on x.\n", size, i);
		exit(EXIT_FAILURE);
	}
	for(a = firsts.y_first, b = a->y_next, i = 1; b; a = b, b = b->y_next, i++) {
		if(a->y > b->y) {
			fprintf(stderr,
				"Not y-sorted at [ ... , %d, %d, ... ], seed %u; :0.\n",
				a->y, b->y, seed);
			exit(EXIT_FAILURE);
		}
	}
	if(i != size) {
		fprintf(stderr, "Input size %u, but output size %u on y.\n", size, i);
		exit(EXIT_FAILURE);
	}

	return (double)(end - begin) / (CLOCKS_PER_SEC / 1000.0);
}

/** Does an experiment to see how new your computer is. */
int main(void) {
	/* look-up table */
	struct TimeData {
		char *const name;
		SortFn sort;
	} const time_data[] = {
		{ "concurrent", &concurrent },
		{ "serial",     &serial },
		{ "qsort",		&qsort_keys }
	}, *td;
	const size_t time_data_size = sizeof time_data / sizeof *time_data;
	unsigned td_i;
	/* experiment */
	FILE *fp = 0, *gnu = 0;
	char fn[32] = "not_a_file";
	unsigned r, samples, base;
	/* error handling */
	enum { E_NO_ERR, E_ERRNO, E_EXTERNAL } error = E_NO_ERR;

	/* seed */
	srand(seed = (unsigned)clock()); rand();

	/* "try-catch-finally" */
	do {
		if(!(gnu = fopen(gnu_name, "w"))) { error = E_ERRNO; break; }
		fprintf(gnu, "set term postscript eps enhanced color\n"
			"set output \"%s\"\n"
			"set grid\n"
			"set xlabel \"size of linked list\"\n"
			"set ylabel \"time, t (ms)\"\n"
			"set yrange [0:]\n"
			"# set xrange [0:1000] # zooming in\n"
			"# seed %u\n"
			"\n"
			"plot", graph_name, seed); /* . . . */
		for(td_i = 0; td = time_data + td_i, td_i < time_data_size; td_i++) {

			/* open <sort>.tsv for writing */
			if(snprintf(fn, sizeof fn, "%s.tsv", td->name) < 0
				|| !(fp = fopen(fn, "w"))) { error = E_ERRNO; break; }
			fprintf(stderr, "Created/overwrote, \"%s,\" to store data on %s "
				"sort.\n", fn, td->name);

			/* do several experiments, increasing number of elements until we
			 hit impatient_ms or foo_capacity is reached */
			fprintf(fp, "# %s: elements ms\n", td->name);
			for(samples = 1, base = 2;
				samples < foo_capacity;
				samples = (unsigned)pow((double)base++, sample_grow_power)) {
				int n = 0;
				double dt_ms = 0.0, mean_ms = 0.0, delta_ms, ssdm = 0.0;

				fprintf(stderr, "%s: sorting %u elements distributed uniformly "
						"at random.\n", td->name, samples);
				/* \cite{Welford1962Note} */
				for(r = 0; r < replicas; r++) {
					fprintf(stderr, "#");
					dt_ms = time_sort_ms(td->sort, samples);
					n++;
					delta_ms = dt_ms - mean_ms;
					mean_ms += delta_ms / n;
					ssdm += delta_ms * (dt_ms - mean_ms);
				}
				fprintf(stderr, " done.\n");
				fprintf(fp, "%u\t%f\t%f\n", samples, mean_ms, sqrt(ssdm / (n - 1)));
				/* loop until the precess is taking too long */
				if(mean_ms >= impatient_ms) break;
			}

			/* close the file */
			if(fclose(fp)) { fp = 0; error = E_ERRNO; break; }
			fp = 0;

			/* write the graph */
			fprintf(gnu,
				"%s\t\"%s\" using 1:2:3 with errorlines lw 3 title \"%s\"",
				td_i ? ", \\\n" : " ", fn, td->name);
		}
	} while(0);
	if(error == E_ERRNO) {
		perror(fn);
	} {
		if(fp  && fclose(fp)) perror(fn);
		if(gnu) {
			fprintf(gnu, "\n");
			if(fclose(gnu)) perror(gnu_name);
		}
	}

	if(error) return EXIT_FAILURE;

	/* it doesn't matter at this point if we fail, but let's try gnuplot? and
	 opening it? */
	error = E_NO_ERR;
	do {
		int s;

		fprintf(stderr, "Running gnuplot to get a graph of, \"%s,\" "
			"(http://www.gnuplot.info/.)\n", gnu_name);

		if(snprintf(fn, sizeof fn, "gnuplot %s", gnu_name) < 0
			|| (s = system(fn)) == -1) { error = E_ERRNO; break; }
		else if(s != EXIT_SUCCESS) { error = E_EXTERNAL; break; }

		fprintf(stderr, "Output, \"%s;\" on a Mac, this opens automatically.\n",
			graph_name);

		if(snprintf(fn, sizeof fn, "open %s", graph_name) < 0
			|| (s = system(fn)) == -1) { error = E_ERRNO; break; }
		else if(s != EXIT_SUCCESS) { error = E_EXTERNAL; break; }

	} while(0);
	if(error == E_ERRNO) {
		perror(fn);
	} else if(error == E_EXTERNAL) {
		fprintf(stderr, "System call, \"%s,\" returned an error; the machine "
			"probably lacks that functionality.\n", fn);
	}

	return EXIT_SUCCESS;
}

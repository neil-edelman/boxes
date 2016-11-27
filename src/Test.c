#include <stdlib.h>	/* EXIT_SUCCESS */
#include <stdio.h>	/* printf */
#include <time.h>	/* clock */
#include <math.h>	/* pow */

struct Foo {
	int x, y;
	struct Foo *x_prev, *x_next;
	struct Foo *y_prev, *y_next;
};

struct FooFirsts {
	struct Foo *x_first, *y_first;
};

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

/* constants */
static const char *const gnu_name   = "sort.p";
static const char *const graph_name = "sort.eps";
static const double impatient_ms    = 500.0;
static const unsigned replicas      = 5;

/* scratch space */
static struct Foo foo[1000000];
static const unsigned foo_capacity = sizeof foo / sizeof *foo;

/* random seed */
static unsigned seed;

/* as long as they remain not-too-large */
static int Foo_compare_x(const struct Foo *a, const struct Foo *b) {
	return a->x - b->x;
}

static int Foo_compare_y(const struct Foo *a, const struct Foo *b) {
	return a->y - b->y;
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

static void concurrent(struct FooFirsts *const this) {
	#pragma omp parallel sections
	{
		#pragma omp section
		this->x_first = FooXLinkSort(this->x_first);
		#pragma omp section
		this->y_first = FooYLinkSort(this->y_first);
	}
}

static void serial(struct FooFirsts *const this) {
	this->x_first = FooXLinkSort(this->x_first);
	this->y_first = FooYLinkSort(this->y_first);
}

static double time_sort_ms(void (*const sort)(struct FooFirsts *const), const unsigned size) {
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
		a->x = rand() / (RAND_MAX / 100.0f);
		a->y = rand() / (RAND_MAX / 100.0f) - 100;
	}

	/* time */
	begin = clock();
	sort(&firsts);
	end   = clock();

	/* test; assumes at least 1 datum */
	for(a = firsts.x_first, b = a->x_next; b; a = b, b = b->x_next) {
		if(a->x > b->x) {
			fprintf(stderr,
				"Not x-sorted at [ ... , %d, %d, ... ], seed %u; :0.\n",
				a->x, b->x, seed);
			exit(EXIT_FAILURE);
		}
	}
	for(a = firsts.y_first, b = a->y_next; b; a = b, b = b->y_next) {
		if(a->y > b->y) {
			fprintf(stderr,
				"Not y-sorted at [ ... , %d, %d, ... ], seed %u; :0.\n",
				a->y, b->y, seed);
			exit(EXIT_FAILURE);
		}
	}

	return (double)(end - begin) / (CLOCKS_PER_SEC / 1000.0);
}

int main(void) {
	/* look-up table */
	struct TimeData {
		const char *const name;
		void (*const sort)(struct FooFirsts *const);
	} const time_data[] = {
		{ "concurrent", &concurrent },
		{ "serial",     &serial }
	}, *td;
	const size_t time_data_size = sizeof time_data / sizeof *time_data;
	unsigned td_i;

	FILE *fp = 0, *gnu = 0;
	char fn[32] = "not_a_file";
	unsigned r, samples, base;

	enum { E_NO_ERR, E_ERRNO, E_EXTERNAL } error = E_NO_ERR;

	/* seed */
	srand(seed = (unsigned)clock()); rand();

	do {
		if(!(gnu = fopen(gnu_name, "w"))) { error = E_ERRNO; break; }
		fprintf(gnu, "set term postscript eps enhanced color\n"
				"set output \"%s\"\n"
				"set grid\n"
				"set xlabel \"array elements\"\n"
				"set ylabel \"time, t (ms)\"\n"
				"set yrange [0:]\n"
				"# set xrange [0:1000] # zooming in\n"
				"# seed %u\n"
				"\n"
				"plot", graph_name, seed);
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
				samples = (unsigned)pow((double)base++, 4.0)) {
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

	/* do a little automation; it doesn't matter at this point if we fail */
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
		fprintf(stderr, "System call, \"%s,\" returned an error.\n", fn);
	}

	return EXIT_SUCCESS;
}

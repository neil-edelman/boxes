#include <unordered_set>
#include <set>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* C++ is a mess. */
extern "C" {
#include "orcish.h"
}

/** @implements <typedef:<int>action_fn> */
static void int_filler(unsigned *x)
	{ *x = (unsigned)rand() / (RAND_MAX / 1000 + 1); }
/** @implements <typedef:<int>to_string_fn> */
static void int_to_string(const unsigned *x, char (*const z)[12])
	{ /*assert(*x < 10000000000),*/ sprintf(*z, "%u", *x); }

#define TREE_NAME o3
#define TREE_ORDER 3
#include "tree.hpp"

#define TREE_NAME o129
#define TREE_ORDER 129
#include "tree.hpp"

#define TREE_NAME o257
#define TREE_ORDER 257
#include "tree.hpp"

#define TREE_NAME o2049
#define TREE_ORDER 2049
#include "tree.hpp"

#include <time.h>
/** Returns a time difference in microseconds from `then`. */
static double diff_us(clock_t then)
	{ return 1000000.0 / CLOCKS_PER_SEC * (clock() - then); }
/** On-line numerically stable first-order statistics, <Welford, 1962, Note>. */
struct measure { size_t count; double mean, ssdm; };
static void m_reset(struct measure *const m)
	{ m->count = 0, m->mean = m->ssdm = 0; }
static void m_add(struct measure *const m, const double replica) {
	const size_t n = ++m->count;
	const double delta = replica - m->mean;
	m->mean += delta / n;
	m->ssdm += delta * (replica - m->mean);
}
static double m_mean(const struct measure *const m)
	{ return m->count ? m->mean : (double)NAN; }
static double m_sample_variance(const struct measure *const m)
	{ return m->count > 1 ? m->ssdm / (m->count - 1) : (double)NAN; }
static double m_stddev(const struct measure *const m)
	{ return sqrt(m_sample_variance(m)); }

#define EXPS \
	X(STD, std), \
	X(O3, o3), \
	X(O128, o128), \
	X(O257, o257), \
	X(O2049, o2049)

int main(void) {
	FILE *gnu = 0;
	const char *name = "timing";
	size_t i, n = 1, e, replicas = 5;
#define X(n, m) n
	enum { EXPS };
#undef X
#define X(n, m) { #m, 0, { 0, 0.0, 0.0 } }
	struct { const char *name; FILE *fp; struct measure m; }
		exp[] = { EXPS };
	const size_t exp_size = sizeof exp / sizeof *exp;
#undef X
	/* Open all graphs for writing. */
	for(e = 0; e < exp_size; e++) {
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", exp[e].name) < 0
			|| !(exp[e].fp = fopen(fn, "w"))) goto catch_;
		fprintf(exp[e].fp, "# %s\n"
			"# <items>\t<t (ms)>\t<sample error on t with %zu replicas>\n",
			exp[e].name, replicas);
	}
	/* Do experiment. */
	for(n = 1; n < 100000/*00*//*0*/; n <<= 1) {
		clock_t t_total;
		size_t r;
		for(e = 0; e < exp_size; e++) m_reset(&exp[e].m);
		for(r = 0; r < replicas; r++) {
			std::set<unsigned/*, void **/> std;
			struct o3_tree o3 = o3_tree();
			struct o129_tree o129 = o129_tree();
			struct o257_tree o257 = o257_tree();
			struct o2049_tree o2049 = o2049_tree();
			clock_t t;
			t_total = clock();
			printf("Replica %lu/%lu.\n", r + 1, replicas);

			t = clock();
			for(i = 0; i < n; i++) std.insert((unsigned)rand());
			m_add(&exp[STD].m, diff_us(t));
			printf("std::set size %zu.\n", std.size());


			/* Set, (closed hash set.) (Don't put I/O in the test.) */
			t = clock();
			for(i = 0; i < n; i++)
				if(!o3_tree_try(&o3, (unsigned)rand())) assert(0), exit(1);
			m_add(&exp[O3].m, diff_us(t));
			/*printf("Tree size %zu: %s.\n",
				o3_tree_count(&o3), o3_tree_to_string(&o3));*/
			printf("Order 3 tree size %zu.\n", o3_tree_count(&o3));

			t = clock();
			for(i = 0; i < n; i++)
				if(!o129_tree_try(&o129, (unsigned)rand())) assert(0), exit(1);
			m_add(&exp[O128].m, diff_us(t));
			/*printf("Tree size %zu: %s.\n",
				o3_tree_count(&o3), o3_tree_to_string(&o3));*/
			printf("Order 129 tree size %zu.\n", o129_tree_count(&o129));

			t = clock();
			for(i = 0; i < n; i++)
				if(!o257_tree_try(&o257, (unsigned)rand())) assert(0), exit(1);
			m_add(&exp[O257].m, diff_us(t));
			printf("Order 257 tree size %zu.\n", o257_tree_count(&o257));

			t = clock();
			for(i = 0; i < n; i++)
				if(!o2049_tree_try(&o2049, (unsigned)rand())) assert(0), exit(1);
			m_add(&exp[O2049].m, diff_us(t));
			printf("Order 2049 tree size %zu.\n", o2049_tree_count(&o2049));

			/* Took took much time; decrease the replicas for next time. */
			if(replicas != 1
				&& 10.0 * (clock() - t_total) / CLOCKS_PER_SEC > 1.0 * replicas)
				replicas--;

			/* Cut a slice to see if it's actually working. */
			/*if(n == 1024) {
				char fn[64];
				sprintf(fn, "graph/%s.gv", exp[0].name);
				set_closed_graph(&closed, fn);
				sprintf(fn, "graph/%s.gv", exp[1].name);
				table_string_graph(&os, fn);
			}*/
			/*closed_set_clear(&closed);
			string_table_clear(&os);*/
			/*closed_set_(&cs);
			string_table_(&os);*/
			o3_tree_(&o3);
			o129_tree_(&o129);
			o257_tree_(&o257);
			o2049_tree_(&o2049);
		}
		for(e = 0; e < exp_size; e++) {
			double stddev = m_stddev(&exp[e].m);
			if(stddev != stddev) stddev = 0; /* Is nan; happens. */
			fprintf(exp[e].fp, "%zu\t%f\t%f\n", n, m_mean(&exp[e].m), stddev);
		}
	}
	goto finally;
catch_:
	perror("timing"), assert(0);
finally:
	for(e = 0; e < exp_size; e++)
		if(exp[e].fp && fclose(exp[e].fp)) perror(exp[e].name);

	/* Output a `gnuplot` script. */
	{
		char fn[64];
		if(sprintf(fn, "graph/%s.gnu", name) < 0
			|| !(gnu = fopen(fn, "w"))) goto catch2;
		fprintf(gnu,
			"set style line 1 lt 5 lw 2 lc rgb '#0072bd'\n"
			"set style line 2 lt 5 lw 2 lc rgb '#ff0000'\n"
			"set style line 3 lt 5 lw 2 lc rgb '#00ac33'\n"
			"set style line 4 lt 5 lw 2 lc rgb '#19d3f5'\n");
		fprintf(gnu, "set term postscript eps enhanced color\n"
			/*"set encoding utf8\n" Doesn't work at all; {/Symbol m}. */
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"elements\"\n"
			"set ylabel \"time per element, t (ns)\"\n"
			"set yrange [0:2000]\n"
			"set log x\n"
			"plot", name);
		for(e = 0; e < exp_size; e++) fprintf(gnu,
			"%s \\\n\"graph/%s.tsv\" using 1:($2/$1*1000):($3/$1*1000) "
			"with errorlines title \"%s\" ls %d", e ? "," : "",
			exp[e].name, exp[e].name, (int)e + 1);
		fprintf(gnu, "\n");
	}
	if(gnu && fclose(gnu)) goto catch2; gnu = 0;
	{
		int result;
		char cmd[64];
		fprintf(stderr, "Running Gnuplot to get a graph of, \"%s,\" "
			"(http://www.gnuplot.info/.)\n", name);
		if((result = system("/usr/local/bin/gnuplot --version")) == -1)
			goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
		if(sprintf(cmd, "/usr/local/bin/gnuplot graph/%s.gnu", name) < 0
			|| (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
		fprintf(stderr, "Running open.\n");
		if(sprintf(cmd, "open graph/%s.eps", name) < 0
		   || (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
	}
	goto finally2;
catch2:
	perror(name);
finally2:
	if(gnu && fclose(gnu)) perror(name);
	printf("\n");
	return EXIT_SUCCESS;
}

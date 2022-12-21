#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#define NAN (0. / 0.)
#endif

#define INSTANCES 100000

struct typical_value { int a, b; };

#define TREE_NAME o3
#define TREE_ORDER 3
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o4
#define TREE_ORDER 4
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o20
#define TREE_ORDER 20
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o40
#define TREE_ORDER 40
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o60
#define TREE_ORDER 60
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o61
#define TREE_ORDER 61
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o62
#define TREE_ORDER 62
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o63
#define TREE_ORDER 63
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o64
#define TREE_ORDER 64
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o65
#define TREE_ORDER 65
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o66
#define TREE_ORDER 66
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o67
#define TREE_ORDER 67
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o77
#define TREE_ORDER 77
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o78
#define TREE_ORDER 78
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o79
#define TREE_ORDER 79
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o80
#define TREE_ORDER 80
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o81
#define TREE_ORDER 81
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o82
#define TREE_ORDER 82
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o83
#define TREE_ORDER 83
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o86
#define TREE_ORDER 86
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o90
#define TREE_ORDER 90
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o99
#define TREE_ORDER 99
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o100
#define TREE_ORDER 100
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o110
#define TREE_ORDER 110
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o129
#define TREE_ORDER 129
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o150
#define TREE_ORDER 150
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o200
#define TREE_ORDER 200
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o257
#define TREE_ORDER 257
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o513
#define TREE_ORDER 513
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o1025
#define TREE_ORDER 1025
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

#define TREE_NAME o2049
#define TREE_ORDER 2049
#define TREE_VALUE struct typical_value *
#include "../../../src/tree.h"

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
	X(O3, o3, 3), \
	X(O4, o4, 4), \
	X(O20, o20, 20), \
	X(O40, o40, 40), \
	X(O60, o60, 60), \
	X(O61, o61, 61), \
	X(O62, o62, 62), \
	X(O63, o63, 63), \
	X(O64, o64, 64), \
	X(O65, o65, 65), \
	X(O66, o66, 66), \
	X(O67, o67, 67), \
	X(O77, o77, 77), \
	X(O78, o78, 78), \
	X(O79, o79, 79), \
	X(O80, o80, 80), \
	X(O81, o81, 81), \
	X(O82, o82, 82), \
	X(O83, o83, 83), \
	X(O90, o90, 90), \
	X(O99, o99, 99), \
	X(O100, o100, 100), \
	X(O110, o110, 110), \
	X(O129, o129, 129), \
	X(O150, o150, 150), \
	X(O200, o200, 200), \
	X(O257, o257, 257), \
	X(O513, o513, 513), \
	X(O1025, o1025, 1025), \
	X(O2049, o2049, 2049)

#define EXP(name) \
static double exp_##name##_us(void) { \
	clock_t t; \
	double dt; \
	size_t i; \
	struct name##_tree tree = name##_tree(); \
	struct typical_value **v; \
	t = clock(); \
	for(i = 0; i < INSTANCES; i++) { \
	if(!name##_tree_try(&tree, (unsigned)rand(), &v)) assert(0), exit(1); \
		*v = 0; \
	} \
	dt = diff_us(t); \
	name##_tree_(&tree); \
	return dt; \
}

/* Could I get this automatic in EXPS? My X-macro skills are lacking. */
EXP(o3)
EXP(o4)
EXP(o20)
EXP(o40)
EXP(o60)
EXP(o61)
EXP(o62)
EXP(o63)
EXP(o64)
EXP(o65)
EXP(o66)
EXP(o67)
EXP(o77)
EXP(o78)
EXP(o79)
EXP(o80)
EXP(o81)
EXP(o82)
EXP(o83)
EXP(o90)
EXP(o99)
EXP(o100)
EXP(o110)
EXP(o129)
EXP(o150)
EXP(o200)
EXP(o257)
EXP(o513)
EXP(o1025)
EXP(o2049)

int main(void) {
	typedef double (*exp_fn)(void);
	FILE *gnu = 0, *fp = 0;
	const char *name = "order";
	const size_t replicas = 12;
	size_t e, r;
#define X(n, m, o) n
	enum { EXPS };
#undef X
#define X(n, m, o) { o, #m, { 0, 0.0, 0.0 }, &exp_ ## m ## _us }
	struct { unsigned order; const char *name; struct measure m; exp_fn fn; }
		exp[] = { EXPS };
	const size_t exp_size = sizeof exp / sizeof *exp;
#undef X
	int ret = EXIT_SUCCESS;
	/* Open graph for writing. */
	{
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", name) < 0
			|| !(fp = fopen(fn, "w"))) goto catch_;
		fprintf(fp, "# <order>\t<items>\t<t (ms)>"
			"\t<sample error on t with %zu replicas>\n", replicas);
	}
	
	/* Do experiment. */
	for(e = 0; e < exp_size; e++) {
		double stddev;
		m_reset(&exp[e].m);
		for(r = 0; r < replicas; r++) {
			double dt = exp[e].fn();
			m_add(&exp[e].m, dt);
			printf("Added %s, replica %lu/%lu: %f.\n", exp[e].name,
				(unsigned long)r + 1, (unsigned long)replicas, dt);
		}
		stddev = m_stddev(&exp[e].m);
		if(stddev != stddev) stddev = 0; /* Is nan; happens. */
		fprintf(fp, "%u\t%f\t%f\n",
			exp[e].order, m_mean(&exp[e].m), stddev);
	}
	goto finally;
catch_:
	perror(name), ret = EXIT_FAILURE;
finally:
	if(fp && fclose(fp)) perror(name);
	/* Output a `gnuplot` script. */
	if(ret == EXIT_SUCCESS) {
		char fn[64];
		if(sprintf(fn, "graph/%s.gnu", name) < 0
			|| !(gnu = fopen(fn, "w"))) goto catch2;
		fprintf(gnu, "set term postscript eps enhanced color\n"
			"set style line 1 lt 5 lw 2 lc rgb '#0072bd'\n"
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"order\"\n"
			"set ylabel \"time to add %u elements, t (ns)\"\n"
			"set xrange [0:500]\n"
			"set yrange [15:25]\n"
			"#set xtics 50\n"
			"plot \"graph/%s.tsv\" using 1:($2/1000):($3/1000) "
			"with errorlines title \"%s\" ls 1\n",
			name, INSTANCES, name, name);
	}
	if(gnu && fclose(gnu)) goto catch2; gnu = 0;
	if(ret == EXIT_SUCCESS) {
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
	return ret;
}

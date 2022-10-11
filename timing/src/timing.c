#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../../test/orcish.h"

struct str16 { char str[16]; };
#define POOL_NAME str16
#define POOL_TYPE struct str16
#include "../../test/pool.h"

#define TRIE_NAME str
#include "../../src/trie.h"

#include <math.h>
#ifndef NAN
#define NAN (0.0/0.0)
#endif
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

int main(int argc, char **argv) {
	const unsigned replicas = 5;
	unsigned order, target, r;
	struct measure m = { 0, 0.0, 0.0 };
	struct str_trie trie = str_trie();
	struct str16_pool str_pool = str16_pool();
	int success = EXIT_SUCCESS;
	{ /* timing order target */
		char *end;
		long l;
		errno = 0;
		if(argc != 3) { errno = EDOM; goto catch; }
		l = strtol(argv[1], &end, 0);
		if(!end || *end || l <= 0 || l >= INT_MAX)
			{ if(!errno) errno = EDOM; goto catch; }
		order = (unsigned)l;
		l = strtol(argv[2], &end, 0);
		if(!end || *end || l <= 0 || l >= INT_MAX)
			{ if(!errno) errno = EDOM; goto catch; }
		target = (unsigned)l;
	}
	fprintf(stderr, "Passed order %u, target %u.\n", order, target);
	/* Do experiment. */
	if(!str16_pool_buffer(&str_pool, 6000)) goto catch;
	m_reset(&m);
	for(r = 0; r < replicas; r++) {
		clock_t t;
		unsigned i;
		struct str16 *orc;
		fprintf(stderr, "Replica %u/%u of %u elements.\n", r + 1, replicas, target);
		t = clock();
		for(i = 0; i < target; i++) {
			if(!(orc = str16_pool_new(&str_pool))) goto catch;
			orcish(orc->str, sizeof orc->str);
			switch(str_trie_try(&trie, orc->str)) {
			case TRIE_ERROR: goto catch;
			case TRIE_UNIQUE: /*printf("%s.\n", orc->str);*/ break;
			case TRIE_PRESENT: /*printf("%s already.\n", orc->str);*/ break;
			}
		}
		m_add(&m, diff_us(t));
		/* Make sure it works {
			struct str_trie_iterator it = str_trie_prefix(&trie, "");
			const char **str;
			while (str = str_trie_next(&it)) fprintf(stderr, "->%s\n", *str);
		}*/
		str_trie_clear(&trie);
		str16_pool_clear(&str_pool);
	}
	{
		double stddev = m_stddev(&m);
		if(stddev != stddev) stddev = 0; /* Is nan; happens. */
		printf(
			/*"# reported order, target, mean (us), stddev (us)\n"*/
			"%u\t%u\t%f\t%f\n", order, target, m_mean(&m), stddev);
	}
	goto finally;
catch:
	success = EXIT_FAILURE;
	perror("timing");
finally:
	return success;
}

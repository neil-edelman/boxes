/* Multiplex shared code. */
#if defined(BOX_NONE) + defined(BOX_ALL) \
	+ defined(BOX_START) + defined(BOX_END) \
	+ defined(BOX_NAME_MISSING) + defined(BOX_NAME_PRESENT) != 1
#	error Request one.
#endif

#ifdef BOX_NONE
#	undef BOX_NONE
#	if defined(BOX_CAT_) || defined(BOX_CAT) || defined(t_) || defined(T_) \
		|| defined(pT_) || defined(tu_) || defined(TU_) || defined(pTU_) \
		/* We know that these are not defined outside, though they can be
		 undefined inside. */ \
		|| defined(BOX_RESTRICT) \
		|| defined(BOX_ENTRY1) || defined(BOX_ENTRY2)
#		error Unexpected preprocessor symbols.
#	endif
#endif

#ifdef BOX_ALL
#	undef BOX_ALL
#	if !defined(BOX_CAT_) || !defined(BOX_CAT) || !defined(t_) || !defined(T_) \
		|| !defined(pT_) || !defined(tu_) || !defined(TU_) || !defined(pTU_) \
		|| !defined(BOX_ENTRY1)
#		error Missing preprocessor symbols.
#	endif
#endif

#ifdef BOX_NAME_MISSING
#	undef BOX_NAME_MISSING
#	if defined(BOX_MINOR_NAME) || defined(BOX_MAJOR_NAME)
#		error Unexpected box name.
#	endif
#endif

#ifdef BOX_NAME_PRESENT
#	undef BOX_NAME_PRESENT
#	if !defined(BOX_MINOR_NAME) || !defined(BOX_MAJOR_NAME)
#		error Missing box name.
#	endif
#endif

#ifdef BOX_START
#	undef BOX_START
/* Allow sub-boxes, but not sub-sub-boxes. I couldn't figure it out. */
#	ifdef BOX_ENTRY1
#		ifdef BOX_ENTRY2
#			error No recursion (yet.)
#		endif
#		define BOX_ENTRY2
#		define BOX_ALL
#		include "box.h"
#	else
#		define BOX_NONE
#		include "box.h"
#		define BOX_ENTRY1
/* <Kernighan and Ritchie, 1988, p. 231>. Allow two levels for containers
 including sub-containers—this is not real recursion. */
#		define BOX_CAT_(n, m) n ## _ ## m
#		define BOX_CAT(n, m) BOX_CAT_(n, m)
#		define t_(n) BOX_CAT(BOX_MINOR_NAME, n)
#		define T_(n) t_(BOX_CAT(BOX_MAJOR_NAME, n))
#		define pT_(n) BOX_CAT(private, T_(n))
#		ifdef BOX_TRAIT
#			define tu_(n) t_(BOX_CAT(BOX_TRAIT, n))
#			define TU_(n) T_(BOX_CAT(BOX_TRAIT, n))
#			define pTU_(n) pT_(BOX_CAT(BOX_TRAIT, n))
#		else /* Anonymous trait. */
#			define tu_(n) t_(n)
#			define TU_(n) T_(n)
#			define pTU_(n) pT_(n)
#		endif
/* Attribute in C99+; ignore otherwise. */
#		if !defined(restrict) && (!defined(__STDC__) \
			|| !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L)
#			define BOX_RESTRICT
#			define restrict
#		endif
#		define BOX_ALL
#		include "box.h"
#	endif
#endif

#ifdef BOX_END
#	undef BOX_END
#	define BOX_ALL
#	include "box.h"
#	ifdef BOX_ENTRY2
#		#undef BOX_ENTRY2
#	else
#		undef BOX_ENTRY1
#		undef BOX_CAT_
#		undef BOX_CAT
#		undef t_
#		undef T_
#		undef pT_
#		undef tu_
#		undef TU_
#		undef pTU_
#		ifdef BOX_RESTRICT
#			undef BOX_RESTRICT
#			undef restrict
#		endif
#		define BOX_NONE
#		include "box.h"
#	endif
#endif

/* Compile with: re2c split.re > split.h */

/*!re2c
	re2c:yyfill:enable = 0;
	re2c:define:YYCTYPE = char;
	re2c:define:YYCURSOR = t;
*/

/**
 @param s: Modified-UTF-8 string.
 @return The first non-classic-whitespace in {s} or a pointer to the
 terminating null if there is no non-whitespace. */
static const char *trim(const char *const s) {
	const char *t = s;
    /*!re2c
		whitespace = [ \t\n\v\f\r];
        *           { return s; }
        whitespace* { return t; }
    */
}

/**
 @param s: Modified-UTF-8 string.
 @return A pointer to the end of the next classic-whitespace delimited word
 in {s} or a null pointer if {s} does not begin at a word. */
static const char *next(const char *const s) {
	const char *t = s;
	/*!re2c
		notwhite = [^ \t\n\v\f\r\x00];
		*         { return 0; }
		notwhite* { return t; }
	*/
}

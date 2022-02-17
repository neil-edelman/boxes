# trie\.h #

Source [src/trie\.h](src/trie.h); examples [test/test\_trie\.c](test/test_trie.c)\.

## Prefix tree ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;value](#user-content-typedef-cc753b30), [&lt;PT&gt;key_fn](#user-content-typedef-1e6e6b3f), [&lt;PT&gt;replace_fn](#user-content-typedef-246bd5da), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812), [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [trie_result](#user-content-tag-eb9850a3), [&lt;T&gt;trie_entry](#user-content-tag-1422bb56), [&lt;T&gt;trie](#user-content-tag-754a10a5)
 * [General Declarations](#user-content-data): [it](#user-content-data-47388410)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of trie.](web/trie.png)

A [&lt;T&gt;trie](#user-content-tag-754a10a5) is a prefix\-tree, digital\-tree, or trie: an ordered set or map of immutable key strings allowing fast prefix queries\. The strings used here are any encoding with a byte null\-terminator, including [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8)\.

The implementation is as [Morrison, 1968 PATRICiA](https://scholar.google.ca/scholar?q=Morrison%2C+1968+PATRICiA): a compact [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only storing the where the key bits are different\. To increase cache\-coherence while allowing for insertion and deletion in &#927;\(\\log `size`\), it uses some B\-tree techniques described in [Bayer, McCreight, 1972 Large](https://scholar.google.ca/scholar?q=Bayer%2C+McCreight%2C+1972+Large)\.



 * Parameter: TRIE\_NAME  
   `<T>` that satisfies `C` naming conventions when mangled\. `<PT>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: TRIE\_VALUE, TRIE\_KEY\_IN\_VALUE  
   `TRIE_VALUE` is an optional payload type to go with the string key\. `TRIE_KEY_IN_VALUE` is an optional [&lt;PT&gt;key_fn](#user-content-typedef-1e6e6b3f) that picks out the key from the of value, otherwise [&lt;PT&gt;entry](#user-content-tag-41052ced) is an associative array entry\.
 * Parameter: TRIE\_TO\_STRING  
   Defining this includes [to\_string\.h](to_string.h), with the keys as the string\.
 * Parameter: TRIE\_TEST  
   Unit testing framework [&lt;T&gt;trie_test](#user-content-fn-ae9d3396), included in a separate header, [\.\./test/test\_trie\.h](../test/test_trie.h)\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e)\. Requires `TRIE_TO_STRING` and that `NDEBUG` not be defined\.
 * Standard:  
   C89
 * Dependancies:  
   [bmp](https://github.com/neil-edelman/bmp)
 * Caveat:  
   ([it](#user-content-data-47388410))


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-cc753b30" name = "user-content-typedef-cc753b30">&lt;PT&gt;value</a> ###

<code>typedef TRIE_VALUE <strong>&lt;PT&gt;value</strong>;</code>

On `TRIE_VALUE`, otherwise just a string\.



### <a id = "user-content-typedef-1e6e6b3f" name = "user-content-typedef-1e6e6b3f">&lt;PT&gt;key_fn</a> ###

<code>typedef const char *(*<strong>&lt;PT&gt;key_fn</strong>)(&lt;PT&gt;entry);</code>

If `TRIE_KEY_IN_VALUE` is set, responsible for picking out the null\-terminated string\.



### <a id = "user-content-typedef-246bd5da" name = "user-content-typedef-246bd5da">&lt;PT&gt;replace_fn</a> ###

<code>typedef int(*<strong>&lt;PT&gt;replace_fn</strong>)(&lt;PT&gt;entry *original, &lt;PT&gt;entry *replace);</code>

A bi\-predicate; returns true if the `replace` replaces the `original`; used in [&lt;T&gt;trie_policy_put](#user-content-fn-50d1d256)\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;type *, char(*)[12]);</code>

[to\_string\.h](to_string.h): responsible for turning the argument into a 12\-`char` null\-terminated output string\. `<PSZ>type` is contracted to be an internal iteration type of the box\.



### <a id = "user-content-typedef-ba462b2e" name = "user-content-typedef-ba462b2e">&lt;PT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PT&gt;action_fn</strong>)(&lt;PT&gt;entry);</code>

Works by side\-effects, _ie_ fills the type with data\. Only defined if `TRIE_TEST`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-eb9850a3" name = "user-content-tag-eb9850a3">trie_result</a> ###

<code>enum <strong>trie_result</strong> { TRIE_RESULT };</code>

A result of modifying the table, of which `TRIE_ERROR` is false\. ![A diagram of the result states.](../web/put.png)



### <a id = "user-content-tag-1422bb56" name = "user-content-tag-1422bb56">&lt;T&gt;trie_entry</a> ###

<code>struct <strong>&lt;T&gt;trie_entry</strong> { const char *key; &lt;PT&gt;value value; } typedef struct &lt;T&gt;trie_entry &lt;PT&gt;entry;</code>

On `TRIE_VALUE` but not `TRIE_KEY_IN_VALUE`, creates a map from key to value as an associative array\. On `TRIE_VALUE` and not `TRIE_KEY_IN_VALUE`, otherwise it's just an alias for [&lt;PT&gt;value](#user-content-typedef-cc753b30)\.



### <a id = "user-content-tag-754a10a5" name = "user-content-tag-754a10a5">&lt;T&gt;trie</a> ###

<code>struct <strong>&lt;T&gt;trie</strong> { struct trie_trunk *root; size_t height; };</code>

To initialize it to an idle state, see [&lt;T&gt;trie](#user-content-fn-754a10a5), `TRIE_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](web/states.png)



## <a id = "user-content-data" name = "user-content-data">General Declarations</a> ##

### <a id = "user-content-data-47388410" name = "user-content-data-47388410">it</a> ###

<code>const static &lt;PT&gt;entry *&lt;PT&gt;next(struct &lt;PT&gt;iterator *const <strong>it</strong>){ assert(it); printf("_next_\n"); if(!it -&gt;trie)return 0; assert(it -&gt;current &amp;&amp;it -&gt;end); if(&amp;it -&gt;current -&gt;trunk ==it -&gt;end){ if(it -&gt;leaf &gt;it -&gt;leaf_end ||it -&gt;leaf &gt;it -&gt;current -&gt;trunk .bsize){ it -&gt;trie = 0; return 0; } } else if(it -&gt;leaf &gt;it -&gt;current -&gt;trunk .bsize){ const char *key = &lt;PT&gt;to_key(it -&gt;current -&gt;leaf[it -&gt;current -&gt;trunk .bsize]); const struct trie_trunk *trunk1 = &amp;it -&gt;current -&gt;trunk; struct trie_trunk *trunk2, *next = 0; size_t h2 = it -&gt;trie -&gt;height, bit2; struct { unsigned br0, br1, lf; } t2; int is_past_end = !it -&gt;end; assert(key); printf("next: %s is the last one on the tree.\n", key); for(it -&gt;current = 0, trunk2 = it -&gt;trie -&gt;root, assert(trunk2), bit2 = 0;; trunk2 = trie_inner(trunk2)-&gt;leaf[t2 .lf]){ int is_considering = 0; if(trunk2 ==trunk1)break; assert(trunk2 -&gt;skip &lt;h2), h2 -=1 +trunk2 -&gt;skip; if(!h2){ printf("next: bailing.\n"); break; } t2 .br0 = 0, t2 .br1 = trunk2 -&gt;bsize, t2 .lf = 0; while(t2 .br0 &lt;t2 .br1){ const struct trie_branch *const branch2 = trunk2 -&gt;branch +t2 .br0; bit2 +=branch2 -&gt;skip; if(!TRIE_QUERY(key, bit2))t2 .br1 = ++t2 .br0 +branch2 -&gt;left; else t2 .br0 +=branch2 -&gt;left +1, t2 .lf +=branch2 -&gt;left +1; bit2 ++; } if(is_past_end){ is_considering = 1; } else if(trunk2 ==it -&gt;end){ is_past_end = 1; if(t2 .lf &lt;it -&gt;leaf_end)is_considering = 1; } if(is_considering &amp;&amp;t2 .lf &lt;trunk2 -&gt;bsize)next = trunk2, it -&gt;leaf = t2 .lf +1, printf("next: continues in tree %s, leaf %u.\n", orcify(trunk2), it -&gt;leaf); } if(!next){ printf("next: fin\n"); it -&gt;trie = 0; return 0; } while(h2)trunk2 = trie_inner_c(trunk2)-&gt;leaf[it -&gt;leaf], it -&gt;leaf = 0, assert(trunk2 -&gt;skip &lt;h2), h2 -=1 +trunk2 -&gt;skip; it -&gt;current = &lt;PT&gt;outer(trunk2); } return it -&gt;current -&gt;leaf +it -&gt;leaf ++; } static void &lt;T&gt;trie(struct &lt;T&gt;trie *const trie){ assert(trie); trie -&gt;root = 0; } static void &lt;T&gt;trie_(struct &lt;T&gt;trie *const trie){ assert(trie); if(trie -&gt;height)&lt;PT&gt;clear(trie -&gt;root, trie -&gt;height); else if(trie -&gt;root)free(trie -&gt;root); &lt;T&gt;trie(trie); } static int &lt;T&gt;trie_from_array(struct &lt;T&gt;trie *const trie, &lt;PT&gt;type *const *const array, const size_t array_size){ return assert(trie &amp;&amp;array &amp;&amp;array_size), &lt;PT&gt;init(trie, array, array_size); } static &lt;PT&gt;entry *&lt;T&gt;trie_match(const struct &lt;T&gt;trie *const trie, const char *const key){ return &lt;PT&gt;match(trie, key); } static int &lt;T&gt;trie_query(struct &lt;N&gt;table *const table, const char *const key, &lt;PN&gt;entry *const result){ struct &lt;PN&gt;bucket *bucket; if(!table ||!table -&gt;buckets ||!(bucket = &lt;PN&gt;query(table, key, &lt;PN&gt;hash(key))))return 0; if(result)&lt;PN&gt;to_entry(bucket, result); return 1; } static &lt;PT&gt;entry &lt;T&gt;trie_get(const struct &lt;T&gt;trie *const trie, const char *const key){ return &lt;PT&gt;get(trie, key); } static &lt;PT&gt;entry *&lt;T&gt;trie_remove(struct &lt;T&gt;trie *const trie, const char *const key){ return &lt;PT&gt;remove(trie, key); } static enum trie_result &lt;T&gt;trie_try(struct &lt;T&gt;trie *const trie, &lt;PT&gt;entry entry){ if(!trie ||!entry)return printf("add: null\n"), TRIE_ERROR; printf("add: trie %s; entry &lt;&lt;%s&gt;&gt;.\n", orcify(trie), &lt;PT&gt;to_key(entry)); return &lt;PT&gt;get(trie, &lt;PT&gt;to_key(entry))?TRIE_YIELD :(&lt;PT&gt;add_unique(trie, entry), TRIE_UNIQUE); } static int &lt;T&gt;trie_put(struct &lt;T&gt;trie *const trie, const &lt;PT&gt;entry x, &lt;PT&gt;entry */*const fixme*/eject){ return assert(trie &amp;&amp;x), &lt;PT&gt;put(trie, x, &amp;eject, 0); } static int &lt;T&gt;trie_policy_put(struct &lt;T&gt;trie *const trie, const &lt;PT&gt;entry x, &lt;PT&gt;entry *eject, const &lt;PT&gt;replace_fn replace){ return assert(trie &amp;&amp;x), &lt;PT&gt;put(trie, x, &amp;eject, replace); } struct &lt;T&gt;trie_iterator { struct &lt;PT&gt;iterator i; };</code>

Advances `it`\. Initializes `trie` to idle\. Returns an initialized `trie` to idle\. Initializes `trie` from an `array` of pointers\-to\-`<T>` of `array_size`\. Tries to remove `key` from `trie`\. Adds a pointer to `x` into `trie` if the key doesn't exist already\. Updates or adds a pointer to `x` into `trie`\. Adds a pointer to `x` to `trie` only if the entry is absent or if calling `replace` returns true or is null\. Stores an iteration range in a trie\. Any changes in the topology of the trie invalidate it\.

 * Caveat:  
   Write this function, somehow\.




## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-b7ff4bcf">&lt;T&gt;trie_size</a></td><td>it</td></tr>

<tr><td align = right>static const &lt;PT&gt;entry *</td><td><a href = "#user-content-fn-f36d1483">&lt;T&gt;trie_next</a></td><td>it</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ae9d3396">&lt;T&gt;trie_test</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-b7ff4bcf" name = "user-content-fn-b7ff4bcf">&lt;T&gt;trie_size</a> ###

<code>static size_t <strong>&lt;T&gt;trie_size</strong>(const struct &lt;T&gt;trie_iterator *const <em>it</em>)</code>

Counts the of the items in the new `it`; iterator must be new, \(calling [&lt;T&gt;trie_next](#user-content-fn-f36d1483) causes it to become undefined\.\)

 * Order:  
   &#927;\(|`it`|\)




### <a id = "user-content-fn-f36d1483" name = "user-content-fn-f36d1483">&lt;T&gt;trie_next</a> ###

<code>static const &lt;PT&gt;entry *<strong>&lt;T&gt;trie_next</strong>(struct &lt;T&gt;trie_iterator *const <em>it</em>)</code>

Advances `it`\.

 * Return:  
   The previous value or null\.




### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things at a time\. `<PSZ>box` is contracted to be the box itself\. `<SZ>` is loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-ae9d3396" name = "user-content-fn-ae9d3396">&lt;T&gt;trie_test</a> ###

<code>static void <strong>&lt;T&gt;trie_test</strong>(void)</code>

Will be tested on stdout\. Requires `TRIE_TEST`, and not `NDEBUG` while defining `assert`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.




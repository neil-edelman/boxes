# table\.h #

Header [\.\./\.\./src/table\.h](../../src/table.h); examples [\.\./\.\./test/test\_table\.c](../../test/test_table.c); article [\.\./table/table\.pdf](../table/table.pdf)\.

## Hash table ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;pT&gt;uint](#user-content-typedef-38271b2b), [&lt;pT&gt;key](#user-content-typedef-95e6d0aa), [&lt;pT&gt;hash_fn](#user-content-typedef-896a1418), [&lt;pT&gt;unhash_fn](#user-content-typedef-25e8a1a1), [&lt;pT&gt;is_equal_fn](#user-content-typedef-f238d00d), [&lt;pT&gt;value](#user-content-typedef-3a465e90), [&lt;pT&gt;entry](#user-content-typedef-9be2614d), [&lt;pT&gt;policy_fn](#user-content-typedef-aafffb12), [&lt;pT&gt;action_fn](#user-content-typedef-348726ce), [&lt;pT&gt;predicate_fn](#user-content-typedef-ad32e23d), [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [table_result](#user-content-tag-4f1ea759), [&lt;T&gt;entry](#user-content-tag-1d176e37), [&lt;t&gt;table](#user-content-tag-2283b713), [&lt;T&gt;cursor](#user-content-tag-43a11ad3), [table_stats](#user-content-tag-89e31bf3)
 * [General Declarations](#user-content-data): [bucket](#user-content-data-f3788f37)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of &lt;string&gt;table.](../../doc/table/table.png)

[&lt;t&gt;table](#user-content-tag-2283b713) implements a set or map of [&lt;pT&gt;entry](#user-content-typedef-9be2614d) as an inline\-chined hash\-table\. It must be supplied [&lt;pT&gt;hash_fn](#user-content-typedef-896a1418) `<t>hash` and, [&lt;pT&gt;is_equal_fn](#user-content-typedef-f238d00d) `<t>is_equal` or [&lt;pT&gt;unhash_fn](#user-content-typedef-25e8a1a1) `<t>unhash`\. It is contiguous and not stable, and may rearrange elements\.



 * Parameter: TABLE\_NAME, TABLE\_KEY  
   `<t>` that satisfies `C` naming conventions when mangled and a valid [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) associated therewith; required\.
 * Parameter: TABLE\_UNHASH  
   By default it assumes that `<t>is_equal` is supplied; with this, instead requires `<t>unhash` satisfying [&lt;pT&gt;unhash_fn](#user-content-typedef-25e8a1a1)\.
 * Parameter: TABLE\_VALUE  
   An optional type that is the payload of the key, thus making this a map or associative array\.
 * Parameter: TABLE\_UINT  
   This is [&lt;pT&gt;uint](#user-content-typedef-38271b2b), the unsigned type of hash of the key given by [&lt;pT&gt;hash_fn](#user-content-typedef-896a1418); defaults to `size_t`\. Usually this can be set to the more sensible value `uint32_t` \(or smaller\) in C99's `stdint.h`\.
 * Parameter: TABLE\_DEFAULT  
   Default trait; a [&lt;pT&gt;value](#user-content-typedef-3a465e90) used in [&lt;T&gt;table&lt;R&gt;get](#user-content-fn-529ef21b)\.
 * Parameter: TABLE\_TO\_STRING  
   To string trait contained in [\.\./\.\./src/to\_string\.h](../../src/to_string.h)\. See [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b)\.
 * Parameter: TABLE\_EXPECT\_TRAIT, TABLE\_TRAIT  
   Named traits are obtained by including `table.h` multiple times with `TABLE_EXPECT_TRAIT` and then subsequently including the name in `TABLE_TRAIT`\.
 * Parameter: TABLE\_DECLARE\_ONLY  
   For headers in different compilation units\.
 * Standard:  
   C89
 * Dependancies:  
   [box](../../src/box.h)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-38271b2b" name = "user-content-typedef-38271b2b">&lt;pT&gt;uint</a> ###

<code>typedef TABLE_UINT <strong>&lt;pT&gt;uint</strong>;</code>

[&lt;pT&gt;hash_fn](#user-content-typedef-896a1418) returns this hash type by `TABLE_UINT`, which must be be an unsigned integer\. Places a simplifying limit on the maximum number of elements of half the cardinality\.



### <a id = "user-content-typedef-95e6d0aa" name = "user-content-typedef-95e6d0aa">&lt;pT&gt;key</a> ###

<code>typedef TABLE_KEY <strong>&lt;pT&gt;key</strong>;</code>

Valid tag type defined by `TABLE_KEY` used for keys\. If `TABLE_UNHASH` is not defined, a copy of this value will be stored in the internal buckets\.



### <a id = "user-content-typedef-896a1418" name = "user-content-typedef-896a1418">&lt;pT&gt;hash_fn</a> ###

<code>typedef &lt;pT&gt;uint(*<strong>&lt;pT&gt;hash_fn</strong>)(const &lt;pT&gt;key);</code>

A map from [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) onto [&lt;pT&gt;uint](#user-content-typedef-38271b2b), called `<t>hash`, that, ideally, should be easy to compute while minimizing duplicate addresses\. Must be consistent for each value while in the table\. If [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) is a pointer, one is permitted to have null in the domain\.



### <a id = "user-content-typedef-25e8a1a1" name = "user-content-typedef-25e8a1a1">&lt;pT&gt;unhash_fn</a> ###

<code>typedef &lt;pT&gt;key(*<strong>&lt;pT&gt;unhash_fn</strong>)(&lt;pT&gt;uint);</code>

Defining `TABLE_UNHASH` says [&lt;pT&gt;hash_fn](#user-content-typedef-896a1418) forms a bijection between the range in [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) and the image in [&lt;pT&gt;uint](#user-content-typedef-38271b2b), and the inverse is called `<t>unhash`\. In this case, keys are not stored in the hash table, rather they are generated using this inverse\-mapping\. \(This provides a smaller and simpler hashing method where the information in the key being hashed is equal to the hash itself—such as numbers\.\)



### <a id = "user-content-typedef-f238d00d" name = "user-content-typedef-f238d00d">&lt;pT&gt;is_equal_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;is_equal_fn</strong>)(const &lt;pT&gt;key a, const &lt;pT&gt;key b);</code>

Equivalence relation between [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) that satisfies `<t>is_equal_fn(a, b) -> <t>hash(a) == <t>hash(b)`, called `<t>is_equal`\. If `TABLE_UNHASH` is set, there is no need for this function because the comparison is done directly in hash space\.



### <a id = "user-content-typedef-3a465e90" name = "user-content-typedef-3a465e90">&lt;pT&gt;value</a> ###

<code>typedef TABLE_VALUE <strong>&lt;pT&gt;value</strong>;</code>

Defining `TABLE_VALUE` produces an associative map, otherwise it is the same as [&lt;pT&gt;key](#user-content-typedef-95e6d0aa)\.



### <a id = "user-content-typedef-9be2614d" name = "user-content-typedef-9be2614d">&lt;pT&gt;entry</a> ###

<code>typedef struct &lt;T&gt;entry <strong>&lt;pT&gt;entry</strong>;</code>

If `TABLE_VALUE`, this is [&lt;T&gt;entry](#user-content-tag-1d176e37); otherwise, it's the same as [&lt;pT&gt;key](#user-content-typedef-95e6d0aa)\.



### <a id = "user-content-typedef-aafffb12" name = "user-content-typedef-aafffb12">&lt;pT&gt;policy_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;policy_fn</strong>)(&lt;pT&gt;key original, &lt;pT&gt;key replace);</code>

Returns true if the `replace` replaces the `original`\. \(fixme: Shouldn't it be entry?\)



### <a id = "user-content-typedef-348726ce" name = "user-content-typedef-348726ce">&lt;pT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;action_fn</strong>)(&lt;pT&gt;type *);</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Operates by side\-effects\.



### <a id = "user-content-typedef-ad32e23d" name = "user-content-typedef-ad32e23d">&lt;pT&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;predicate_fn</strong>)(const &lt;pT&gt;type *);</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Returns a boolean given read\-only\.



### <a id = "user-content-typedef-4442127b" name = "user-content-typedef-4442127b">&lt;pT&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;to_string_fn</strong>)(const &lt;pT&gt;key, const &lt;pT&gt;value *, char(*)[12]);</code>

The type of the required `<tr>to_string`\. Responsible for turning the read\-only argument into a 12\-max\-`char` output string\. `<pT>value` is omitted when it's a set\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-4f1ea759" name = "user-content-tag-4f1ea759">table_result</a> ###

<code>enum <strong>table_result</strong> { TABLE_RESULT };</code>

A result of modifying the table, of which `TABLE_ERROR` is false\.

![A diagram of the result states.](../../doc/table/result.png)



### <a id = "user-content-tag-1d176e37" name = "user-content-tag-1d176e37">&lt;T&gt;entry</a> ###

<code>struct <strong>&lt;T&gt;entry</strong> { &lt;pT&gt;key key; &lt;pT&gt;value value; };</code>

Defining `TABLE_VALUE` creates this map from [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) to [&lt;pT&gt;value](#user-content-typedef-3a465e90), as an interface with table\.



### <a id = "user-content-tag-2283b713" name = "user-content-tag-2283b713">&lt;t&gt;table</a> ###

<code>struct <strong>&lt;t&gt;table</strong> { struct &lt;pT&gt;bucket *buckets; &lt;pT&gt;uint log_capacity, size, top; };</code>

To initialize, see [&lt;t&gt;table](#user-content-fn-2283b713), `TABLE_IDLE`, `{0}` \(`C99`,\) or being `static`\. The fields should be treated as read\-only; any modification is liable to cause the table to go into an invalid state\.

![States.](../../doc/table/states.png)



### <a id = "user-content-tag-43a11ad3" name = "user-content-tag-43a11ad3">&lt;T&gt;cursor</a> ###

<code>struct <strong>&lt;T&gt;cursor</strong> { struct &lt;t&gt;table *table; &lt;pT&gt;uint i; };</code>

![States](../../doc/table/it.png)

Adding, deleting, successfully looking up entries, or any modification of the table's topology invalidates the iterator\. Iteration usually not in any particular order, but deterministic up to topology changes\. The asymptotic runtime of iterating though the whole table is proportional to the capacity\.



### <a id = "user-content-tag-89e31bf3" name = "user-content-tag-89e31bf3">table_stats</a> ###

<code>struct <strong>table_stats</strong> { size_t n, max; double mean, ssdm; };</code>

[Welford1962Note](https://scholar.google.ca/scholar?q=Welford1962Note): population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`\.



## <a id = "user-content-data" name = "user-content-data">General Declarations</a> ##

### <a id = "user-content-data-f3788f37" name = "user-content-data-f3788f37">bucket</a> ###

<code>... static &lt;pT&gt;key &lt;pT&gt;bucket_key(const struct &lt;pT&gt;bucket *const <strong>bucket</strong>){ assert(bucket &amp;&amp;bucket -&gt;next !=TABLE_NULL); return &lt;t&gt;unhash(bucket -&gt;hash); return bucket -&gt;key; } static &lt;pT&gt;value &lt;pT&gt;bucket_value(const struct &lt;pT&gt;bucket *const bucket){ assert(bucket &amp;&amp;bucket -&gt;next !=TABLE_NULL); return bucket -&gt;value; return &lt;pT&gt;bucket_key(bucket); } static &lt;pT&gt;uint &lt;pT&gt;capacity(const struct &lt;t&gt;table *const table){ return assert(table &amp;&amp;table -&gt;buckets &amp;&amp;table -&gt;log_capacity &gt;=3),(&lt;pT&gt;uint)((&lt;pT&gt;uint)1 &lt;&lt;table -&gt;log_capacity); } static &lt;pT&gt;uint &lt;pT&gt;chain_head(const struct &lt;t&gt;table *const table, const &lt;pT&gt;uint hash){ return hash &amp;(&lt;pT&gt;capacity(table)-1); } static struct &lt;pT&gt;bucket *&lt;pT&gt;prev(const struct &lt;t&gt;table *const table, const &lt;pT&gt;uint b){ const struct &lt;pT&gt;bucket *const bucket = table -&gt;buckets +b; &lt;pT&gt;uint to_next = TABLE_NULL, next; assert(table &amp;&amp;bucket -&gt;next !=TABLE_NULL); for(next = &lt;pT&gt;chain_head(table, bucket -&gt;hash); next !=b; to_next = next, next = table -&gt;buckets[next].next); return to_next !=TABLE_NULL ?table -&gt;buckets +to_next :0; } static void &lt;pT&gt;grow_stack(struct &lt;t&gt;table *const table){ &lt;pT&gt;uint top =(table -&gt;top &amp;~TABLE_HIGH)-!(table -&gt;top &amp;TABLE_HIGH); assert(table &amp;&amp;table -&gt;buckets &amp;&amp;table -&gt;top &amp;&amp;top &lt;&lt;pT&gt;capacity(table)); while(table -&gt;buckets[top].next !=TABLE_NULL)assert(top), top --; table -&gt;top = top; } static void &lt;pT&gt;force_stack(struct &lt;t&gt;table *const table){ &lt;pT&gt;uint top = table -&gt;top; if(top &amp;TABLE_HIGH){ struct &lt;pT&gt;bucket *bucket; top &amp;=~TABLE_HIGH; do bucket = table -&gt;buckets +++top/*, assert(top &lt; capacity)*/; while(bucket -&gt;next !=TABLE_NULL &amp;&amp;&lt;pT&gt;chain_head(table, bucket -&gt;hash)==top); table -&gt;top = top; } } static int &lt;pT&gt;in_stack_range(const struct &lt;t&gt;table *const table, const &lt;pT&gt;uint i){ return assert(table &amp;&amp;table -&gt;buckets),(table -&gt;top &amp;~TABLE_HIGH)+!!(table -&gt;top &amp;TABLE_HIGH)&lt;=i; } static void &lt;pT&gt;shrink_stack(struct &lt;t&gt;table *const table, const &lt;pT&gt;uint b){ assert(table &amp;&amp;table -&gt;buckets &amp;&amp;b &lt;&lt;pT&gt;capacity(table)); assert(table -&gt;buckets[b].next ==TABLE_NULL); if(!&lt;pT&gt;in_stack_range(table, b))return; &lt;pT&gt;force_stack(table); assert(&lt;pT&gt;in_stack_range(table, b)); if(b !=table -&gt;top){ struct &lt;pT&gt;bucket *const prev = &lt;pT&gt;prev(table, table -&gt;top); table -&gt;buckets[b]= table -&gt;buckets[table -&gt;top]; prev -&gt;next = b; } table -&gt;buckets[table -&gt;top].next = TABLE_NULL; table -&gt;top |=TABLE_HIGH; } static void &lt;pT&gt;move_to_top(struct &lt;t&gt;table *const table, const &lt;pT&gt;uint m){ struct &lt;pT&gt;bucket *move, *top, *prev; assert(table &amp;&amp;table -&gt;size &lt;&lt;pT&gt;capacity(table)&amp;&amp;m &lt;&lt;pT&gt;capacity(table)); &lt;pT&gt;grow_stack(table); move = table -&gt;buckets +m, top = table -&gt;buckets +table -&gt;top; assert(move -&gt;next !=TABLE_NULL &amp;&amp;top -&gt;next ==TABLE_NULL); if(prev = &lt;pT&gt;prev(table, m))prev -&gt;next = table -&gt;top; memcpy(top, move, sizeof *move), move -&gt;next = TABLE_NULL; } static int &lt;pT&gt;equal_buckets(const &lt;pT&gt;key a, const &lt;pT&gt;key b){ return(void)a,(void)b, 1; return &lt;t&gt;is_equal(a, b); } static struct &lt;pT&gt;bucket *&lt;pT&gt;query(struct &lt;t&gt;table *const table, /*pT_(key_c)*/const &lt;pT&gt;key key, const &lt;pT&gt;uint hash){ struct &lt;pT&gt;bucket *bucket1; &lt;pT&gt;uint head, b0 = TABLE_NULL, b1, b2; assert(table &amp;&amp;table -&gt;buckets &amp;&amp;table -&gt;log_capacity); bucket1 = table -&gt;buckets +(head = b1 = &lt;pT&gt;chain_head(table, hash)); if((b2 = bucket1 -&gt;next)==TABLE_NULL ||&lt;pT&gt;in_stack_range(table, b1)&amp;&amp;b1 !=&lt;pT&gt;chain_head(table, bucket1 -&gt;hash))return 0; while(hash !=bucket1 -&gt;hash ||!&lt;pT&gt;equal_buckets(key, &lt;pT&gt;bucket_key(bucket1))){ if(b2 ==TABLE_END)return 0; bucket1 = table -&gt;buckets +(b0 = b1, b1 = b2); assert(b1 &lt;&lt;pT&gt;capacity(table)&amp;&amp;&lt;pT&gt;in_stack_range(table, b1)&amp;&amp;b1 !=TABLE_NULL); b2 = bucket1 -&gt;next; } return bucket1; if(b0 ==TABLE_NULL)return bucket1; { struct &lt;pT&gt;bucket *const bucket0 = table -&gt;buckets +b0, *const bucket_head = table -&gt;buckets +head, temp; bucket0 -&gt;next = b2; memcpy(&amp;temp, bucket_head, sizeof *bucket_head); memcpy(bucket_head, bucket1, sizeof *bucket1); memcpy(bucket1, &amp;temp, sizeof temp); bucket_head -&gt;next = b1; return bucket_head; } } static int &lt;pT&gt;buffer(struct &lt;t&gt;table *const table, const &lt;pT&gt;uint n){ struct &lt;pT&gt;bucket *buckets; const &lt;pT&gt;uint log_c0 = table -&gt;log_capacity, c0 = log_c0 ?(&lt;pT&gt;uint)((&lt;pT&gt;uint)1 &lt;&lt;log_c0):0; &lt;pT&gt;uint log_c1, c1, size1, i, wait, mask; assert(table &amp;&amp;table -&gt;size &lt;=TABLE_HIGH &amp;&amp;(!table -&gt;buckets &amp;&amp;!table -&gt;size &amp;&amp;!log_c0 &amp;&amp;!c0 ||table -&gt;buckets &amp;&amp;table -&gt;size &lt;=c0 &amp;&amp;log_c0 &gt;=3)); if(TABLE_M1 -table -&gt;size &lt;n ||TABLE_HIGH &lt;(size1 = table -&gt;size +n))return errno = ERANGE, 0; if(table -&gt;buckets)log_c1 = log_c0, c1 = c0 ?c0 :1; else log_c1 = 3, c1 = 8; while(c1 &lt;size1)log_c1 ++, c1 &lt;&lt;=1; if(log_c0 ==log_c1)return 1; if(!(buckets = realloc(table -&gt;buckets, sizeof *buckets *c1))){ if(!errno)errno = ERANGE; return 0; } table -&gt;top =(c1 -1)|TABLE_HIGH; table -&gt;buckets = buckets, table -&gt;log_capacity = log_c1; { struct &lt;pT&gt;bucket *e = buckets +c0, *const e_end = buckets +c1; for(; e &lt;e_end; e ++)e -&gt;next = TABLE_NULL; } mask =(&lt;pT&gt;uint)((((&lt;pT&gt;uint)1 &lt;&lt;log_c0)-1)^(((&lt;pT&gt;uint)1 &lt;&lt;log_c1)-1)); wait = TABLE_END; for(i = 0; i &lt;c0; i ++){ struct &lt;pT&gt;bucket *idx, *go; &lt;pT&gt;uint g, hash; idx = table -&gt;buckets +i; if(idx -&gt;next ==TABLE_NULL)continue; g = &lt;pT&gt;chain_head(table, hash = idx -&gt;hash); if(i ==g){ idx -&gt;next = TABLE_END; continue; } if((go = table -&gt;buckets +g)-&gt;next ==TABLE_NULL){ struct &lt;pT&gt;bucket *head; &lt;pT&gt;uint h = g &amp;~mask; assert(h &lt;=g); if(h &lt;g &amp;&amp;i &lt;h &amp;&amp;(head = table -&gt;buckets +h, assert(head -&gt;next !=TABLE_NULL), &lt;pT&gt;chain_head(table, head -&gt;hash)==g)){ memcpy(go, head, sizeof *head); go -&gt;next = TABLE_END, head -&gt;next = TABLE_NULL; } else { memcpy(go, idx, sizeof *idx); go -&gt;next = TABLE_END, idx -&gt;next = TABLE_NULL; continue; } } idx -&gt;next = wait, wait = i; } { &lt;pT&gt;uint prev = TABLE_END, w = wait; while(w !=TABLE_END){ struct &lt;pT&gt;bucket *waiting = table -&gt;buckets +w; &lt;pT&gt;uint cl = &lt;pT&gt;chain_head(table, waiting -&gt;hash); struct &lt;pT&gt;bucket *const closed = table -&gt;buckets +cl; assert(cl !=w); if(closed -&gt;next ==TABLE_NULL){ memcpy(closed, waiting, sizeof *waiting), closed -&gt;next = TABLE_END; if(prev !=TABLE_END)table -&gt;buckets[prev].next = waiting -&gt;next; if(wait ==w)wait = waiting -&gt;next; w = waiting -&gt;next, waiting -&gt;next = TABLE_NULL; } else { assert(closed -&gt;next ==TABLE_END); prev = w, w = waiting -&gt;next; } } } while(wait !=TABLE_END){ struct &lt;pT&gt;bucket *const waiting = table -&gt;buckets +wait; &lt;pT&gt;uint h = &lt;pT&gt;chain_head(table, waiting -&gt;hash); struct &lt;pT&gt;bucket *const head = table -&gt;buckets +h; struct &lt;pT&gt;bucket *top; assert(h !=wait &amp;&amp;head -&gt;next !=TABLE_NULL); &lt;pT&gt;grow_stack(table), top = table -&gt;buckets +table -&gt;top; memcpy(top, waiting, sizeof *waiting); top -&gt;next = head -&gt;next, head -&gt;next = table -&gt;top; wait = waiting -&gt;next, waiting -&gt;next = TABLE_NULL; } return 1; } static void &lt;pT&gt;replace_key(struct &lt;pT&gt;bucket *const bucket, const &lt;pT&gt;key key, const &lt;pT&gt;uint hash){(void)key; bucket -&gt;hash = hash; bucket -&gt;key = key; } static struct &lt;pT&gt;bucket *&lt;pT&gt;evict(struct &lt;t&gt;table *const table, const &lt;pT&gt;uint hash){ &lt;pT&gt;uint i; struct &lt;pT&gt;bucket *bucket; if(!&lt;pT&gt;buffer(table, 1))return 0; bucket = table -&gt;buckets +(i = &lt;pT&gt;chain_head(table, hash)); if(bucket -&gt;next !=TABLE_NULL){ int in_stack = &lt;pT&gt;chain_head(table, bucket -&gt;hash)!=i; &lt;pT&gt;move_to_top(table, i); bucket -&gt;next = in_stack ?TABLE_END :table -&gt;top; } else { bucket -&gt;next = TABLE_END; } table -&gt;size ++; return bucket; } static enum table_result &lt;pT&gt;put_key(struct &lt;t&gt;table *const table, const &lt;pT&gt;key key, &lt;pT&gt;key *eject, const &lt;pT&gt;policy_fn policy){ struct &lt;pT&gt;bucket *bucket; const &lt;pT&gt;uint hash = &lt;t&gt;hash(key); enum table_result result; assert(table); if(table -&gt;buckets &amp;&amp;(bucket = &lt;pT&gt;query(table, key, hash))){ if(!policy ||!policy(&lt;pT&gt;bucket_key(bucket), key))return TABLE_PRESENT; if(eject)*eject = &lt;pT&gt;bucket_key(bucket); result = TABLE_PRESENT; } else { if(!(bucket = &lt;pT&gt;evict(table, hash)))return TABLE_ERROR; result = TABLE_ABSENT; } &lt;pT&gt;replace_key(bucket, key, hash); return result; } static struct &lt;T&gt;cursor &lt;T&gt;begin(const struct &lt;t&gt;table *const table){ union { const struct &lt;t&gt;table *readonly; struct &lt;t&gt;table *promise; } sly; struct &lt;T&gt;cursor cur; cur .table =(sly .readonly = table, sly .promise), cur .i = 0; return cur; } static int &lt;T&gt;exists(struct &lt;T&gt;cursor *const cur){ const struct &lt;t&gt;table *t; &lt;pT&gt;uint limit; if(!cur ||!(t = cur -&gt;table)||!cur -&gt;table -&gt;buckets/* Idle */)return 0; limit = &lt;pT&gt;capacity(t); while(cur -&gt;i &lt;limit){ if(t -&gt;buckets[cur -&gt;i].next !=TABLE_NULL)return 1; cur -&gt;i ++; } cur -&gt;table = 0; return 0; } static struct &lt;pT&gt;bucket *&lt;T&gt;entry(const struct &lt;T&gt;cursor *const cur){ return cur -&gt;table -&gt;buckets +cur -&gt;i; } static &lt;pT&gt;key &lt;T&gt;key(const struct &lt;T&gt;cursor *const cur){ return &lt;pT&gt;bucket_key(cur -&gt;table -&gt;buckets +cur -&gt;i); } static &lt;pT&gt;value *&lt;T&gt;value(const struct &lt;T&gt;cursor *const cur){ return &amp;cur -&gt;table -&gt;buckets[cur -&gt;i].value; } static void &lt;T&gt;next(struct &lt;T&gt;cursor *const cur){ cur -&gt;i ++; } static int &lt;T&gt;cursor_remove(struct &lt;T&gt;cursor *const cur){ struct &lt;t&gt;table *table = cur -&gt;table; struct &lt;pT&gt;bucket *previous = 0, *current; &lt;pT&gt;uint prv = TABLE_NULL, crnt; assert(cur &amp;&amp;table); if(!cur -&gt;table -&gt;buckets)return 0; assert(cur -&gt;i &lt;&lt;pT&gt;capacity(cur -&gt;table)); if(cur -&gt;i &gt;=&lt;pT&gt;capacity(cur -&gt;table))return 0; current = cur -&gt;table -&gt;buckets +cur -&gt;i, assert(current -&gt;next !=TABLE_NULL); crnt = &lt;pT&gt;chain_head(cur -&gt;table, current -&gt;hash); while(crnt !=cur -&gt;i)assert(crnt &lt;&lt;pT&gt;capacity(cur -&gt;table)), crnt =(previous = cur -&gt;table -&gt;buckets +(prv = crnt))-&gt;next; if(prv !=TABLE_NULL){ previous -&gt;next = current -&gt;next; } else if(current -&gt;next !=TABLE_END){ const &lt;pT&gt;uint scnd = current -&gt;next; struct &lt;pT&gt;bucket *const second = table -&gt;buckets +scnd; assert(scnd &lt;&lt;pT&gt;capacity(table)); memcpy(current, second, sizeof *second); if(crnt &lt;scnd)cur -&gt;i --; crnt = scnd, current = second; } current -&gt;next = TABLE_NULL, table -&gt;size --, &lt;pT&gt;shrink_stack(table, crnt); return 1; } static struct &lt;t&gt;table &lt;t&gt;table(void){ struct &lt;t&gt;table table; table .buckets = 0; table .log_capacity = 0; table .size = 0; table .top = 0; return table; } static void &lt;t&gt;table_(struct &lt;t&gt;table *const table){ if(table)free(table -&gt;buckets), *table = &lt;t&gt;table(); } static int &lt;T&gt;buffer(struct &lt;t&gt;table *const table, const &lt;pT&gt;uint n){ return assert(table), &lt;pT&gt;buffer(table, n); } static void &lt;T&gt;clear(struct &lt;t&gt;table *const table){ struct &lt;pT&gt;bucket *b, *b_end; assert(table); if(!table -&gt;buckets){ assert(!table -&gt;log_capacity); return; } assert(table -&gt;log_capacity); for(b = table -&gt;buckets, b_end = b +&lt;pT&gt;capacity(table); b &lt;b_end; b ++)b -&gt;next = TABLE_NULL; table -&gt;size = 0; table -&gt;top =(&lt;pT&gt;capacity(table)-1)|TABLE_HIGH; } static int &lt;T&gt;contains(struct &lt;t&gt;table *const table, const &lt;pT&gt;key key){ return table &amp;&amp;table -&gt;buckets ?!!&lt;pT&gt;query(table, key, &lt;t&gt;hash(key)):0; } static int &lt;T&gt;query(struct &lt;t&gt;table *const table, const &lt;pT&gt;key key, &lt;pT&gt;key *result, &lt;pT&gt;value *value){ struct &lt;pT&gt;bucket *bucket; if(!table ||!table -&gt;buckets ||!(bucket = &lt;pT&gt;query(table, key, &lt;t&gt;hash(key))))return 0; if(result)*result = &lt;pT&gt;bucket_key(bucket); if(value)*value = bucket -&gt;value; return 1; } static int &lt;T&gt;query(struct &lt;t&gt;table *const table, const &lt;pT&gt;key key, &lt;pT&gt;key *result){ struct &lt;pT&gt;bucket *bucket; if(!table ||!table -&gt;buckets ||!(bucket = &lt;pT&gt;query(table, key, &lt;t&gt;hash(key))))return 0; if(result)*result = &lt;pT&gt;bucket_key(bucket); return 1; } static &lt;pT&gt;value &lt;T&gt;get_or(struct &lt;t&gt;table *const table, const &lt;pT&gt;key key, &lt;pT&gt;value default_value){ struct &lt;pT&gt;bucket *bucket; return table &amp;&amp;table -&gt;buckets &amp;&amp;(bucket = &lt;pT&gt;query(table, key, &lt;t&gt;hash(key)))?&lt;pT&gt;bucket_value(bucket):default_value; } static enum table_result &lt;T&gt;try(struct &lt;t&gt;table *const table, &lt;pT&gt;key key){ return &lt;pT&gt;put_key(table, key, 0, 0); } static enum table_result &lt;pT&gt;assign(struct &lt;t&gt;table *const table, &lt;pT&gt;key key, &lt;pT&gt;value **const content){ struct &lt;pT&gt;bucket *bucket; const &lt;pT&gt;uint hash = &lt;t&gt;hash(key); enum table_result result; assert(table &amp;&amp;content); if(table -&gt;buckets &amp;&amp;(bucket = &lt;pT&gt;query(table, key, hash))){ result = TABLE_PRESENT; } else { if(!(bucket = &lt;pT&gt;evict(table, hash)))return TABLE_ERROR; &lt;pT&gt;replace_key(bucket, key, hash); result = TABLE_ABSENT; } *content = &amp;bucket -&gt;value; return result; } static enum table_result &lt;T&gt;assign(struct &lt;t&gt;table *const table, &lt;pT&gt;key key, &lt;pT&gt;value **const content){ return &lt;pT&gt;assign(table, key, content); } static int &lt;pT&gt;always_replace(const &lt;pT&gt;key original, const &lt;pT&gt;key replace){ return(void)original,(void)replace, 1; } static enum table_result &lt;T&gt;update(struct &lt;t&gt;table *const table, &lt;pT&gt;key key, &lt;pT&gt;key *eject){ return &lt;pT&gt;put_key(table, key, eject, &amp;&lt;pT&gt;always_replace); } static enum table_result &lt;T&gt;policy(struct &lt;t&gt;table *const table, &lt;pT&gt;key key, &lt;pT&gt;key *eject, const &lt;pT&gt;policy_fn policy){ return &lt;pT&gt;put_key(table, key, eject, policy); } static int &lt;T&gt;remove(struct &lt;t&gt;table *const table, const &lt;pT&gt;key key){ struct &lt;pT&gt;bucket *current; &lt;pT&gt;uint c, p = TABLE_NULL, n, hash = &lt;t&gt;hash(key); if(!table ||!table -&gt;size)return 0; assert(table -&gt;buckets); current = table -&gt;buckets +(c = &lt;pT&gt;chain_head(table, hash)); if((n = current -&gt;next)==TABLE_NULL ||&lt;pT&gt;in_stack_range(table, c)&amp;&amp;c !=&lt;pT&gt;chain_head(table, current -&gt;hash))return 0; while(hash !=current -&gt;hash &amp;&amp;!&lt;pT&gt;equal_buckets(key, &lt;pT&gt;bucket_key(current))){ if(n ==TABLE_END)return 0; p = c, current = table -&gt;buckets +(c = n); assert(c &lt;&lt;pT&gt;capacity(table)&amp;&amp;&lt;pT&gt;in_stack_range(table, c)&amp;&amp;c !=TABLE_NULL); n = current -&gt;next; } if(p !=TABLE_NULL){ struct &lt;pT&gt;bucket *previous = table -&gt;buckets +p; previous -&gt;next = current -&gt;next; } else if(current -&gt;next !=TABLE_END){ struct &lt;pT&gt;bucket *const second = table -&gt;buckets +(c = current -&gt;next); assert(current -&gt;next &lt;&lt;pT&gt;capacity(table)); memcpy(current, second, sizeof *second); current = second; } current -&gt;next = TABLE_NULL, table -&gt;size --, &lt;pT&gt;shrink_stack(table, c); return 1; } static void &lt;pT&gt;unused_base_coda(void);</code>

Gets the key of an occupied `bucket`\. Gets the value of an occupied `bucket`, which might be the same as the key\. The capacity of a non\-idle `table` is always a power\-of\-two\. On return, the `top` of `table` will be empty and eager, but size is not incremented, leaving it in intermediate state\. Amortized if you grow only\. Force the evaluation of the stack of `table`, thereby making it eager\. This is like searching for a bucket in open\-addressing\. Is `i` in `table` possibly on the stack? \(The stack grows from the high\.\) Corrects newly\-deleted `b` from `table` in the stack\. Moves the `m` index in non\-idle `table`, to the top of collision stack\. This may result in an inconsistent state; one is responsible for filling that hole and linking it with top\. `TABLE_UNHASH` is injective, so in that case, we only compare hashes\. `table` will be searched linearly for `key` which has `hash`\. Ensures that `table` has enough buckets to fill `n` more than the size\. May invalidate and re\-arrange the order\. Replace the `key` and `hash` of `bucket`\. Don't touch next\. Evicts the spot where `hash` goes in `table`\. This results in a space in the table\. Put `key` in `table`\. For collisions, only if `policy` exists and returns true do and displace it to `eject`, if non\-null\. Move to next on `cur` that exists\. Removes the entry at `cur`\. Whereas [&lt;T&gt;remove](#user-content-fn-56806709) invalidates the cursor, this corrects `cur` so [&lt;T&gt;next](#user-content-fn-d0790d04) is the next entry\. To use the cursor after this, one must move to the next\. Zeroed data \(not all\-bits\-zero\) is initialized\. If `table` is not null, destroys and returns it to idle\. Reserve at least `n` more empty buckets in `table`\. This may cause the capacity to increase and invalidates any pointers to data in the table\. Clears and removes all buckets from `table`\. The capacity and memory of the `table` is preserved, but all previous values are un\-associated\. \(The load factor will be less until it reaches it's previous size\.\) If can be no default key, use this to separate a null—returns false—from a result\. Otherwise, a more convenient function is [&lt;T&gt;get_or](#user-content-fn-e9879d51)\. `key` from `table` is stored in `result`\. Only if `TABLE_VALUE` is not set; see [&lt;T&gt;assign](#user-content-fn-40416930) for a map\. Puts `key` in set `table` only if absent\. Only if `TABLE_VALUE` is set\. Ensures that `key` is in the `table` and update `content`\. Only if `TABLE_VALUE` is set; see [&lt;T&gt;try](#user-content-fn-edcfce52) for a set\. Puts `key` in the map `table` and store the associated value in `content`\. Callback in [&lt;T&gt;update](#user-content-fn-5772e298)\. Puts `key` in `table`, replacing an equal\-valued key\. \(If keys are indistinguishable, this function is not very useful, see [&lt;T&gt;try](#user-content-fn-edcfce52) or [&lt;T&gt;assign](#user-content-fn-40416930)\.\) Puts `key` in `table` only if absent or if calling `policy` returns true\. Removes `key` from `table` \(which could be null\.\)

 * Implements:  
   [&lt;pT&gt;policy_fn](#user-content-typedef-aafffb12)




## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>struct &lt;T&gt;cursor</td><td><a href = "#user-content-fn-80df50b2">&lt;T&gt;begin</a></td><td>&lt;t&gt;table</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-dd6c86e1">&lt;T&gt;exists</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>&lt;pT&gt;type *</td><td><a href = "#user-content-fn-1d176e37">&lt;T&gt;entry</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-d0790d04">&lt;T&gt;next</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>&lt;pT&gt;type *</td><td><a href = "#user-content-fn-443f2b31">&lt;TR&gt;any</a></td><td>&lt;pT&gt;box, &lt;pTR&gt;predicate_fn</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-51d87ca4">&lt;TR&gt;each</a></td><td>&lt;pT&gt;box, &lt;pTR&gt;action_fn</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-21ef106e">&lt;TR&gt;if_each</a></td><td>&lt;pT&gt;box, &lt;pTR&gt;predicate_fn, &lt;pTR&gt;action_fn</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a></td><td>restrict, restrict, &lt;pTR&gt;predicate_fn</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-443f2b31">&lt;TR&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-51d87ca4">&lt;TR&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-21ef106e">&lt;TR&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a></td><td>dst, src, copy</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a76df7bd">&lt;TR&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-4e047ffb">&lt;T&gt;graph</a></td><td>&lt;pT&gt;box</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-6c32bc30">&lt;T&gt;graph_fn</a></td><td>&lt;pT&gt;box, char</td></tr>

<tr><td align = right>static &lt;pT&gt;value</td><td><a href = "#user-content-fn-529ef21b">&lt;T&gt;table&lt;R&gt;get</a></td><td>table, key</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-80df50b2" name = "user-content-fn-80df50b2">&lt;T&gt;begin</a> ###

<code>struct &lt;T&gt;cursor <strong>&lt;T&gt;begin</strong>(const struct <em>&lt;t&gt;table</em> *);</code>



### <a id = "user-content-fn-dd6c86e1" name = "user-content-fn-dd6c86e1">&lt;T&gt;exists</a> ###

<code>int <strong>&lt;T&gt;exists</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-1d176e37" name = "user-content-fn-1d176e37">&lt;T&gt;entry</a> ###

<code>&lt;pT&gt;type *<strong>&lt;T&gt;entry</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-d0790d04" name = "user-content-fn-d0790d04">&lt;T&gt;next</a> ###

<code>void <strong>&lt;T&gt;next</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-443f2b31" name = "user-content-fn-443f2b31">&lt;TR&gt;any</a> ###

<code>&lt;pT&gt;type *<strong>&lt;TR&gt;any</strong>(const <em>&lt;pT&gt;box</em> *, <em>&lt;pTR&gt;predicate_fn</em>);</code>



### <a id = "user-content-fn-51d87ca4" name = "user-content-fn-51d87ca4">&lt;TR&gt;each</a> ###

<code>void <strong>&lt;TR&gt;each</strong>(<em>&lt;pT&gt;box</em> *, <em>&lt;pTR&gt;action_fn</em>);</code>



### <a id = "user-content-fn-21ef106e" name = "user-content-fn-21ef106e">&lt;TR&gt;if_each</a> ###

<code>void <strong>&lt;TR&gt;if_each</strong>(<em>&lt;pT&gt;box</em> *, <em>&lt;pTR&gt;predicate_fn</em>, <em>&lt;pTR&gt;action_fn</em>);</code>



### <a id = "user-content-fn-f61ec8de" name = "user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a> ###

<code>int <strong>&lt;TR&gt;copy_if</strong>(&lt;pT&gt;box *<em>restrict</em>, const &lt;pTR&gt;box *<em>restrict</em>, <em>&lt;pTR&gt;predicate_fn</em>);</code>



### <a id = "user-content-fn-443f2b31" name = "user-content-fn-443f2b31">&lt;TR&gt;any</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;TR&gt;any</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-51d87ca4" name = "user-content-fn-51d87ca4">&lt;TR&gt;each</a> ###

<code>static void <strong>&lt;TR&gt;each</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;action_fn <em>action</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Iterates through `box` and calls `action` on all the elements\. Differs calling `action` until the iterator is one\-ahead, so can delete elements as long as it doesn't affect the next, \(specifically, a linked\-list\.\)

 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`action`\)




### <a id = "user-content-fn-21ef106e" name = "user-content-fn-21ef106e">&lt;TR&gt;if_each</a> ###

<code>static void <strong>&lt;TR&gt;if_each</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>, const &lt;pTR&gt;action_fn <em>action</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; \(&#927;\(`predicate`\) \+ &#927;\(`action`\)\)




### <a id = "user-content-fn-f61ec8de" name = "user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a> ###

<code>static int <strong>&lt;TR&gt;copy_if</strong>(&lt;pT&gt;box *restrict const <em>dst</em>, const &lt;pTR&gt;box *restrict const <em>src</em>, const &lt;pTR&gt;predicate_fn <em>copy</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h), `BOX_CONTIGUOUS`: For all elements of `src`, calls `copy`, and if true, lazily copies the elements to `dst`\. `dst` and `src` can not be the same but `src` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: realloc  
 * Order:  
   &#927;\(|`src`|\) &#215; &#927;\(`copy`\)




### <a id = "user-content-fn-8bb1c0a2" name = "user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a> ###

<code>static void <strong>&lt;TR&gt;keep_if</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>keep</em>, const &lt;pTR&gt;action_fn <em>destruct</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h) `BOX_CONTIGUOUS`: For all elements of `box`, calls `keep`, and if false, if contiguous, lazy deletes that item, if not, eagerly\. Calls `destruct` if not\-null before deleting\.

 * Order:  
   &#927;\(|`box`|\) \(&#215; O\(`keep`\) \+ O\(`destruct`\)\)




### <a id = "user-content-fn-a76df7bd" name = "user-content-fn-a76df7bd">&lt;TR&gt;trim</a> ###

<code>static void <strong>&lt;TR&gt;trim</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h), `BOX_CONTIGUOUS`: Removes at either end of `box` the things that `predicate`, if it exists, returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-260f8348" name = "user-content-fn-260f8348">&lt;TR&gt;to_string</a> ###

<code>const char *<strong>&lt;TR&gt;to_string</strong>(const &lt;pT&gt;box *const <em>box</em>);</code>



### <a id = "user-content-fn-260f8348" name = "user-content-fn-260f8348">&lt;TR&gt;to_string</a> ###

<code>static const char *<strong>&lt;TR&gt;to_string</strong>(const &lt;pT&gt;box *const <em>box</em>)</code>

[\.\./\.\./src/to\_string\.h](../../src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things in a single sequence point\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-4e047ffb" name = "user-content-fn-4e047ffb">&lt;T&gt;graph</a> ###

<code>void <strong>&lt;T&gt;graph</strong>(const <em>&lt;pT&gt;box</em> *, FILE *);</code>



### <a id = "user-content-fn-6c32bc30" name = "user-content-fn-6c32bc30">&lt;T&gt;graph_fn</a> ###

<code>int <strong>&lt;T&gt;graph_fn</strong>(const <em>&lt;pT&gt;box</em> *, const <em>char</em> *);</code>



### <a id = "user-content-fn-529ef21b" name = "user-content-fn-529ef21b">&lt;T&gt;table&lt;R&gt;get</a> ###

<code>static &lt;pT&gt;value <strong>&lt;T&gt;table&lt;R&gt;get</strong>(struct &lt;t&gt;table *const <em>table</em>, const &lt;pT&gt;key <em>key</em>)</code>

This is functionally identical to [&lt;T&gt;get_or](#user-content-fn-e9879d51), but a with a trait specifying a constant default value\. This is the most convenient access method, but it needs to have a `TABLE_DEFAULT`\.

 * Return:  
   The value associated with `key` in `table`, \(which can be null\.\) If no such value exists, the `TABLE_DEFAULT` for this trait is returned\.
 * Order:  
   Average &#927;\(1\); worst &#927;\(n\)\.






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2019 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.




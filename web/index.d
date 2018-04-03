A wrapper around the standard <em>C89</em> string library functions
with a dynamic array. Using <a href =
"https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8">Modified
UTF-8</a>. Text is currently undergoing splitting into <em>Text</em>
and <em>String</em> to not be a monolithic array that has
<em>O(n)</em> performance for large strings. Sorry for the renaming
everything; this will probably not work.

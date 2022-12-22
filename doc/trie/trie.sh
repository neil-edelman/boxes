#!/bin/sh
pdflatex -shell-escape trie.tex

# check . . . should write a script, or an re2c programme
# \usepackage[pdf,tmpdir]{graphviz} % [singlefile] to use > ~16
# ...
# \begin{figure}
#	\centering
#	\subcaptionbox{Bit view.\label{star-3:bits}}{
#		\digraph[scale=0.6]{star3bits}{
# ...
#		}
#	}
# }
# digraph is removed
# \detokenize{UTF-8}
# blank lines are removed
# comments are removed
# no non-alphanumeric in base filename

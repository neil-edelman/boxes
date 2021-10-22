graph := ../graph
tar := targets
graphs = colour-3-add-logic.gv.tex #colour-4-split-mem.gv.tex

default: $(tar)/$(graphs)
	# . . . success

$(tar)/%.gv.tex: $(graph)/%.gv
	@mkdir targets
	dot2tex -o $@ $<

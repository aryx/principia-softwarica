
graph:
	~/pfff/codegraph -symlinks -derived_data -lang clang2 -build .

visual:
	~/pfff/codemap -no_legend -symlinks -filter cpp -ss 2 .


trace:
	mk clean
	mk libs > make_trace.txt
	mk kernels >> make_trace.txt
	mk cmds >> make_trace.txt

clangfiles:
	~/pfff/pfff_test -analyze_make_trace make_trace.txt > compile_commands.json
#	~/pfff/pfff -gen_clang compile_commands.json
#	~/pfff/pfff_test.opt -uninclude_clang

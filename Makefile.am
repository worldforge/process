SUBDIRS = src perf loadtool

test:
	./src/process
	./src/process -r > regress.output
	diff regress.output regress.default

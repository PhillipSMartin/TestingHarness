.PHONY: test

test/%:
	build/TestingHarness -t lh-test -C -f $*.txt 
all:
	$(MAKE) -f bex.mak
	$(MAKE) -f cex.mak
	$(MAKE) -f gex.mak
	$(MAKE) -f hex.mak
	$(MAKE) -f qex.mak
	$(MAKE) -f mex.mak
tmake:
	tmake -nodepend bex -o bex.mak
	tmake -nodepend cex -o cex.mak
	tmake -nodepend gex -o gex.mak
	tmake -nodepend hex -o hex.mak
	tmake -nodepend qex -o qex.mak
	tmake -nodepend mex -o qex.mak
clean:
	$(MAKE) -f bex.mak clean
	$(MAKE) -f cex.mak clean
	$(MAKE) -f gex.mak clean
	$(MAKE) -f hex.mak clean
	$(MAKE) -f qex.mak clean
	$(MAKE) -f mex.mak clean

#############################################################################
# Qt Makefile - doc
#############################################################################
# $Id: //depot/qt/main/doc/Makefile#3 $

showfiles showsources:
	@echo *.doc

clean:
	-rm -f *~ *% #*

# 'make checkout' updates files that MUST be old
# 'make checkout CODATE=-d1994/12/24' recreates 1994/12/24
# 'make checkout CODATE=-d2000/1/1 updates all files
checkout:
	@for rcs in RCS/*,v ; do \
	    file=`basename $$rcs ,v` ;\
	    [ -n "${CODATE}" ] || \
	    [ '!' -f $$file ] || \
	    ( [ '!' -w $$file ] && [ $$rcs -nt $$file ] ) && \
	    co ${CODATE} $$file ;\
	done; \
	/bin/true	# tell make it worked

obj include depend:

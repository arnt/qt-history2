EXTENSIONS=xt nsplugin

EXAMPLES=\
	nsplugin/examples/grapher \
	nsplugin/examples/qtimage \
	nsplugin/examples/trivial \
	\
	xt/examples/mainlyMotif \
	xt/examples/mainlyQt \
	xt/examples/mainlyXt \


extensions:
	for i in $(EXTENSIONS); do \
	    if ( cd $$i/src; $(MAKE) ); then \
		true; \
	    else \
		break; \
	    fi; \
	done

examples:
	for i in $(EXAMPLES); do \
	    if ( cd $$i; $(MAKE) ); then \
		true; \
	    else \
		break; \
	    fi; \
	done

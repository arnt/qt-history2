EXTENSIONS=imageio opengl xt nsplugin

EXAMPLES=\
	nsplugin/examples/grapher \
	nsplugin/examples/qtimage \
	nsplugin/examples/trivial \
	\
	opengl/examples/box \
	opengl/examples/gear \
	opengl/examples/sharedbox \
	\
	xt/examples/mainlyMotif \
	xt/examples/mainlyQt \
	xt/examples/mainlyXt \
	


extensions:
	for i in $(EXTENSIONS); do \
	    if ( cd $$i; $(MAKE) ); then \
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

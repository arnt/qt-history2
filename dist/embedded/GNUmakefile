SUBDIRS = 		src \
                examples

all: bin/moc $(SUBDIRS) 
 
$(SUBDIRS): include FORCE
	cd $@; $(MAKE)
 
bin/moc:  include FORCE 
	cd src/moc; $(MAKE); cp moc ../../bin

clean:
	for i in $(SUBDIRS); do ( cd $$i ; $(MAKE) clean ) ; done
 
include: FORCE
	syncqt
 
FORCE: 


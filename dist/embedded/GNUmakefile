SUBDIRS = 	src \
                examples

QT_EMBEDDED = 1
export QT_EMBEDDED

QTDIR = $(PWD)
export QTDIR


all: bin/moc $(SUBDIRS) 

$(SUBDIRS): include FORCE
	cd $@; $(MAKE)

bin/moc:  include FORCE 
	cd src/moc; $(MAKE); cp moc ../../bin

clean:
	for i in $(SUBDIRS); do ( cd $$i ; $(MAKE) clean ) ; done

include: FORCE
	./bin/syncqt

FORCE: 


#############################################################################
# Qt Global Makefile
#############################################################################

#######	Sub directories

ARCHS	=	linux-shared	\
		solaris-shared-gcc
DIRS	=	src examples tutorial

#######	Build rules

all: checkarch
	@a=`cat .arch`; echo "Hey, we're making it now"
	@set -e ; for d in $(DIRS); do cd $$d; make clean; cd ..; done

$(ARCHS):
	@echo "Making architecture $@..."
	@echo $@ > .arch
	@cd  arch/$@ && make

checkarch: .arch

.arch:
	@echo "You must first create makefiles for your architecture:"
	@echo $(ARCHS) | fmt -1 | sed -e 's/^/  make /'
	@exit 1


clean:
	-rm -f .arch
	@set -e ; for d in $(DIRS); do cd $$d; make clean; cd ..; done

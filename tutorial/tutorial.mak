# $Source: /tmp/cvs/qt/tutorial/Attic/tutorial.mak,v $
#

####### Directories

SUBDIRS =	t1 \
		t2 \
		t3 \
		t4 \
		t5 \
		t6 \
		t7 \
		t8 \
		t9 \
		t10 \
		t11 \
		t12 \
		t13 \
		t14

####### Targets

all: targets

showdirs:
	@echo $(SUBDIRS)

targets:
	cd t1
	$(MAKE) -f t1.mak
	cd ..
	cd t2
	$(MAKE) -f t2.mak
	cd ..
	cd t3
	$(MAKE) -f t3.mak
	cd ..
	cd t4
	$(MAKE) -f t4.mak
	cd ..
	cd t5
	$(MAKE) -f t5.mak
	cd ..
	cd t6
	$(MAKE) -f t6.mak
	cd ..
	cd t7
	$(MAKE) -f t7.mak
	cd ..
	cd t8
	$(MAKE) -f t8.mak
	cd ..
	cd t9
	$(MAKE) -f t9.mak
	cd ..
	cd t10
	$(MAKE) -f t10.mak
	cd ..
	cd t11
	$(MAKE) -f t11.mak
	cd ..
	cd t12
	$(MAKE) -f t12.mak
	cd ..
	cd t13
	$(MAKE) -f t13.mak
	cd ..
	cd t14
	$(MAKE) -f t14.mak
	cd ..

galore:
	cd t1
	t1.exe
	cd ..
	cd t2
	t2.exe
	cd ..
	cd t3
	t3.exe
	cd ..
	cd t4
	t4.exe
	cd ..
	cd t5
	t5.exe
	cd ..
	cd t6
	t6.exe
	cd ..
	cd t7
	t7.exe
	cd ..
	cd t8
	t8.exe
	cd ..
	cd t9
	t9.exe
	cd ..
	cd t10
	t10.exe
	cd ..
	cd t11
	t11.exe
	cd ..
	cd t12
	t12.exe
	cd ..
	cd t13
	t13.exe
	cd ..
	cd t14
	t14.exe
	cd ..

tmake:
	cd t1
	tmake t1.pro -o t1.mak
	cd ..
	cd t2
	tmake t2.pro -o t2.mak
	cd ..
	cd t3
	tmake t3.pro -o t3.mak
	cd ..
	cd t4
	tmake t4.pro -o t4.mak
	cd ..
	cd t5
	tmake t5.pro -o t5.mak
	cd ..
	cd t6
	tmake t6.pro -o t6.mak
	cd ..
	cd t7
	tmake t7.pro -o t7.mak
	cd ..
	cd t8
	tmake t8.pro -o t8.mak
	cd ..
	cd t9
	tmake t9.pro -o t9.mak
	cd ..
	cd t10
	tmake t10.pro -o t10.mak
	cd ..
	cd t11
	tmake t11.pro -o t11.mak
	cd ..
	cd t12
	tmake t12.pro -o t12.mak
	cd ..
	cd t13
	tmake t13.pro -o t13.mak
	cd ..
	cd t14
	tmake t14.pro -o t14.mak
	cd ..

clean:
	cd t1
	$(MAKE) -f t1.mak clean
	cd ..
	cd t2
	$(MAKE) -f t2.mak clean
	cd ..
	cd t3
	$(MAKE) -f t3.mak clean
	cd ..
	cd t4
	$(MAKE) -f t4.mak clean
	cd ..
	cd t5
	$(MAKE) -f t5.mak clean
	cd ..
	cd t6
	$(MAKE) -f t6.mak clean
	cd ..
	cd t7
	$(MAKE) -f t7.mak clean
	cd ..
	cd t8
	$(MAKE) -f t8.mak clean
	cd ..
	cd t9
	$(MAKE) -f t9.mak clean
	cd ..
	cd t10
	$(MAKE) -f t10.mak clean
	cd ..
	cd t11
	$(MAKE) -f t11.mak clean
	cd ..
	cd t12
	$(MAKE) -f t12.mak clean
	cd ..
	cd t13
	$(MAKE) -f t13.mak clean
	cd ..
	cd t14
	$(MAKE) -f t14.mak clean
	cd ..

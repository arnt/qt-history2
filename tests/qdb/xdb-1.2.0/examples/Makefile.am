
INCLUDES= -I$(topdir) $(all_includes)
LDADD	= -L$(topdir)/xdb/.libs -lxdb

# for shadow passwords override the value
INSTALL_PROGRAM = @INSTALL@

noinst_PROGRAMS = sample1 sample2 sample3 sample4 sample5 testdate      \
               testhtml.cgi indextst testdbinfo

sample1_SOURCES  = sample1.cpp
sample2_SOURCES  = sample2.cpp
sample3_SOURCES  = sample3.cpp
sample4_SOURCES  = sample4.cpp
sample5_SOURCES  = sample5.cpp
indextst_SOURCES = indextst.cpp
testdate_SOURCES = testdate.cpp
testhtml_cgi_SOURCES = testhtml.cpp
testdbinfo_SOURCES = testdbinfo.cpp

noinst_HEADERS =

MAINTAINERCLEANFILES = Makefile.in

TEMPLATE = subdirs

CONFIG 	+= ordered

QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"

SUBDIRS	 = basicbrowsing \
	   basicbrowsing2 \
	   basicdatamanip \
	   connect1 \
	   create_connections \
	   custom1 \
	   delete \
	   extract \
	   form1 \
	   form2 \
	   insert \
	   insert2 \
	   navigating \
	   order1 \
	   order2 \
	   retrieve1 \
	   retrieve2 \
	   subclass1 \
	   subclass2 \
	   subclass3 \
	   subclass4 \
	   subclass5 \
	   table1 \
	   table2 \
	   table3 \
	   table4 \
	   update

targets = chap-preface.sgml \
	chap-tutorial-preface.sgml \
	chap-one.sgml \
	chap-main-windows.sgml \
	chap-subclassing.sgml \
	chap-advanced-widgets.sgml \
	chap-customizing.sgml \
	chap-custom-controls.sgml \
	chap-creating-database-applications.sgml \
	chap-06.sgml \
	chap-07.sgml \
	chap-08.sgml \
	manual.sgml \
	tutorial.sgml

manual: $(targets)

ps: manual.ps

html: manual.html

tuthtml: tutorial.html

tutorial: tutorial.ps

manual.ps: $(targets)
	sgmltools -b ps manual.sgml > /dev/null 
	
tutorial.ps: $(targets)
	sgmltools -b ps tutorial.sgml > /dev/null 

tutorial.html: $(targets)
	sgmltools -b html tutorial.sgml > /dev/null 
	./fiximg tutorial/*.html
	#p4 edit ../*.html
	#mv tutorial/*.html ../


manual.html: $(targets)
	sgmltools -b html manual.sgml > /dev/null 
	./fiximg manual/*.html
	p4 edit ../*.html
	mv manual/*.html ../

clean:
	rm -f $(targets) *.ps

%.sgml: %.sgml.mpp
	mpp -m 0 $^


targets = chap-preface.sgml \
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
	manual.sgml

manual: $(targets)

ps: manual.ps

html: manual.html

manual.ps: $(targets)
	sgmltools -b ps manual.sgml > /dev/null 
	
manual.html: $(targets)
	sgmltools -b html manual.sgml > /dev/null 
	./fiximg manual/*.html
	p4 edit ../*.html
	mv manual/*.html ../

clean:
	rm -f $(targets) *.ps

%.sgml: %.sgml.mpp
	mpp -m 0 $^


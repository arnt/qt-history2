targets = chap-preface.sgml \
	chap-one.sgml \
	chap-two.sgml \
	chap-02.sgml \
	chap-03.sgml \
	chap-04.sgml \
	chap-advanced-widgets.sgml \
	chap-customizing.sgml \
	chap-main-windows.sgml \
	chap-custom-controls.sgml \
	chap-creating-database-applications.sgml \
	chap-06.sgml \
	chap-07.sgml \
	chap-08.sgml \
	chap-09.sgml \
	chap-10.sgml \
	chap-useful-techniques.sgml \
	manual.sgml

manual: $(targets)

ps: manual.ps

manual.ps: $(targets)
	sgmltools -b ps manual.sgml > /dev/null 
	
clean:
	rm -f $(targets) *.ps

%.sgml: %.sgml.mpp
	mpp -m 0 $^


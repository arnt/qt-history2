targets = chap-preface.sgml \
	chap-01.sgml \
	chap-02.sgml \
	chap-03.sgml \
	chap-04.sgml \
	chap-05.sgml \
	chap-06.sgml \
	chap-07.sgml \
	chap-08.sgml \
	chap-09.sgml \
	chap-10.sgml \
	chap-custom-controls.sgml \
	chap-creating-database-applications.sgml \
	manual.sgml

manual: $(targets)

ps: manual.ps

manual.ps: $(targets)
	sgmltools -b ps manual.sgml > /dev/null 
	
clean:
	rm -f $(targets) *.ps

%.sgml: %.sgml.mpp
	mpp -m 0 $^


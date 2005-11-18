TEMPLATE = app
LANGUAGE = C++

FORMS += project.ui

TRANSLATIONS        = project_no.ts.tmp
win32: RES = $$system(copy /Y project_no.ts project_no.ts.tmp)
unix: RES = $$system(cp -f project_no.ts project_no.ts.tmp)

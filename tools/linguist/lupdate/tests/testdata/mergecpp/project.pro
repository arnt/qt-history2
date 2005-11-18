TEMPLATE = app
LANGUAGE = C++

SOURCES += finddialog.cpp

TRANSLATIONS        = project_no.ts.tmp

# Copy the ts to a temp file because:
# 1. The depot file is usually read-only
# 2. We don't want to modify the original file, since then it won't be possible to run the test twice
#    without reverting the original file again.

win32: RES = $$system(copy /Y project_no.ts $$TRANSLATIONS)
unix: RES = $$system(cp -f project_no.ts $$TRANSLATIONS)

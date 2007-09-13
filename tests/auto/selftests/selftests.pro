TEMPLATE = subdirs

SUBDIRS = subtest test warnings maxwarnings cmptest globaldata skipglobal skip \
          strcmp expectfail sleep fetchbogus crashes multiexec failinit failinitdata \
          skipinit skipinitdata datetime singleskip assert waitwithoutgui differentexec \
          exception qexecstringlist

INSTALLS =

QT = core

DEFINES += QT_USE_USING_NAMESPACE


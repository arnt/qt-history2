TEMPLATE = subdirs
SUBDIRS = testProcessCrash \
          testProcessEcho \
          testProcessEcho2 \
          testProcessEcho3 \
          testProcessLoopback \
	  testProcessNormal \
          testProcessOutput \
          testProcessDeadWhileReading \
          testProcessEOF \
	  testSoftExit \
          testProcessSpacesArgs/nospace.pro \
          testProcessSpacesArgs/onespace.pro \
          testProcessSpacesArgs/twospaces.pro \
          testExitCodes \
          testSpaceInName

win32 {
          SUBDIRS += testProcessEchoGui
}

SUBDIRS += test

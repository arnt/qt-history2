CONFIG = qt thread

#count
!count( CONFIG, 2 ) {
   message( "FAILED: count function: $$CONFIG" )
}

#contains
!contains( CONFIG, thread ) {
   message( "FAILED: contains function: $$CONFIG" )
}

#exists
!exists( functions.pro ) {
   message( "FAILED: exists function" )
}

#isEmpty
isEmpty( CONFIG ) {
   message( "FAILED: isEmpty function: $CONFIG" )
}

#infile
!infile( infiletest.pro, DEFINES, QT_DLL ){
   message( "FAILED: infile function" )
}

#include
include( infiletest.pro )
!contains( DEFINES, QT_DLL ) {
   message( "FAILED: include function: $$DEFINES" )
}

lessThan(QT_VERSION, 40200) {
    message( "SKIPPED: replace function only in 4.2" )
} else {
    #replace
    VERSION=1.0.0
    VERSION_replaced=$$replace(VERSION,\.,_)
    !isEqual(VERSION_replaced, 1_0_0) {
       message( "FAILED: replace function: $$VERSION_replaced" )
    }
}

#test functions
defineTest(myTestFunction) {
   RESULT =
   list=$$1
   for(l, list) {
       RESULT += $$l
   }
   export(RESULT)
}
myTestFunction(oink baa moo)
!equals($$list($$member(RESULT, 0)), "oink") {
     message("FAILED: myTestFunction: $$RESULT")
}
myTestFunction("oink baa" moo)
!equals($$list($$member(RESULT, 0)), "oink baa") {
     message("FAILED: myTestFunction: $$RESULT")
}
myTestFunction(oink "baa moo")
!equals($$list($$member(RESULT, 0)), "oink") {
     message("FAILED: myTestFunction: $$RESULT")
}
myTestFunction("oink baa moo")
!equals($$list($$member(RESULT, 0)), "oink baa moo") {
     message("FAILED: myTestFunction: $$RESULT")
}

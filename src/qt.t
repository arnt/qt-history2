#$ IncludeTemplate("lib.t");

#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libfreetype.a');
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libmng.a');
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libpng.a');
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libz.a');
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libjpeg.a');

#$ Substitute('tmp/qt.cpp: ../include/qt.h $$ALLHEADERS');
	echo "#include <qt.h>" >tmp/qt.cpp
	$(CXX) -E -DQT_MOC_CPP $(CXXFLAGS) $(INCPATH) >tmp/moc_qt.h tmp/qt.cpp
	$(MOC) -o tmp/qt.cpp tmp/moc_qt.h
	perl -pi -e 's/"moc_qt.h"/"qt.h"/' tmp/qt.cpp

#$ Substitute('$$DESTDIR_TARGET: $$OBJECTS_DIR/libfreetype.a $$OBJECTS_DIR/libz.a $$OBJECTS_DIR/libpng.a $$OBJECTS_DIR/libmng.a $$OBJECTS_DIR/libjpeg.a');

#$ Substitute('$$OBJECTS_DIR/libfreetype.a:');
#$ Substitute('	cd 3rdparty/freetype2; make CONFIG_MK=config$$DASHMIPS.mk OBJ_DIR=../../$$OBJECTS_DIR ../../$$OBJECTS_DIR/libfreetype.a');

#$ Substitute('$$OBJECTS_DIR/libz.a:');
#$ Substitute('	cd 3rdparty/zlib; make -f Makefile$$DASHMIPS; cp libz.a ../../$$OBJECTS_DIR');

#$ Substitute('$$OBJECTS_DIR/libpng.a:');
#$ Substitute('	cd 3rdparty/libpng; make -f scripts/makefile.linux$$DASHMIPS; cp libpng.a ../../$$OBJECTS_DIR');

#$ Substitute('$$OBJECTS_DIR/libmng.a:');
#$ Substitute('	cd 3rdparty/libmng; make -f makefiles/makefile.linux$$DASHMIPS; cp libmng.a ../../$$OBJECTS_DIR');

#$ Substitute('$$OBJECTS_DIR/libjpeg.a:');
#$ Substitute('	cd 3rdparty/jpeglib; make -f makefile.unix$$DASHMIPS; cp libjpeg.a ../../$$OBJECTS_DIR');

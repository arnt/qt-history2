#$ IncludeTemplate("lib.t");

#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libfreetype.a');
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libmng.a') if Config("mng");
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libpng.a') if Config("png");
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libz.a') if Config("png");
#$ Substitute('LIBS += '.$project{"OBJECTS_DIR"}.'/libjpeg.a') if Config("jpeg");

#$ Substitute('tmp/qt.cpp: ../include/qt.h $$ALLHEADERS');
	echo "#include <qt.h>" >tmp/qt.cpp
	$(CXX) -E -DQT_MOC_CPP $(CXXFLAGS) $(INCPATH) >tmp/moc_qt.h tmp/qt.cpp
	$(MOC) -o tmp/qt.cpp tmp/moc_qt.h
	perl -pi -e 's/"moc_qt.h"/"qt.h"/' tmp/qt.cpp

#${
    $l = '$$OBJECTS_DIR/libfreetype.a';
    $l .= ' $$OBJECTS_DIR/libz.a' if Config("png");
    $l .= ' $$OBJECTS_DIR/libmng.a' if Config("mng");
    $l .= ' $$OBJECTS_DIR/libpng.a' if Config("png");
    $l .= ' $$OBJECTS_DIR/libjpeg.a' if Config("jpeg");
    
    Substitute('$$DESTDIR_TARGET: '.$l );
#$}

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

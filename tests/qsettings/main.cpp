#include <qapplication.h>
#include "qsettings.h"
#include <qvariant.h>
#include <qimage.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qiconset.h>
#include <qpointarray.h>
#include <qcursor.h>
#include <qcursor.h>
#include <qdom.h>

#ifdef Q_OS_UNIX
#include <sys/time.h>
#else
#include <time.h>
#endif


int main(int argc, char **argv) {
    QApplication app(argc, argv);

    {
	qDebug("== testing all QVariant data types");

	QSettings settings;
	QSettings settings2;
	QSettings settings3;

	settings.setWritable(TRUE);

	settings.setPath(QSettings::Unix, "variantrc");
	settings2.setPath(QSettings::Unix, "variantrc2");
	settings3.setPath(QSettings::Unix, "variantrc3");

	settings.setFallback(&settings2);
	settings2.setFallback(&settings3);

	// String
	QString vstring = "this is a string";
	settings.writeEntry("/variant/string/string", vstring);

       	// StringList
	QStringList vstrlist;
	vstrlist.append("foo");
	vstrlist.append("bar");
	vstrlist.append("baz");
	vstrlist.append("b0rk");
	settings.writeEntry("/variant/string/list", vstrlist);

	// Font
	QFont vfont;
	settings.writeEntry("/variant/font/font", vfont);

	// Image
	QImage vimage;
	vimage.load("qtlogo.png");
	settings.writeEntry("/variant/image/image", vimage);

	// Pixmap
	QPixmap vpixmap;
	vpixmap = vimage;
	settings.writeEntry("/variant/image/pixmap", vpixmap);

	// Bitmap
	QBitmap vbitmap;
	vbitmap = vimage.convertDepth(1);
	settings.writeEntry("/variant/image/bitmap", vbitmap);

	// Color
	QColor vcolor(192, 192, 176);
	settings.writeEntry("/variant/painter/color", vcolor);

	// Brush
	QBrush vbrush;
	vbrush.setColor(vcolor);
	vbrush.setPixmap(vpixmap);
	settings.writeEntry("/variant/painter/brush", vbrush);

	// Rect
	QRect vrect(424, 242, 42, 42);
	settings.writeEntry("/variant/dimensions/rect", vrect);

	// Size
	QSize vsize(666, 666);
	settings.writeEntry("/variant/dimensions/size", vsize);

	// Palette
	QPalette vpalette;
	settings.writeEntry("/variant/painter/palette", vpalette);

	// ColorGroup
	settings.writeEntry("/variant/palette/colorgroup", vpalette.active());

	// IconSet
	QIconSet viconset;
	viconset.setPixmap(vpixmap, QIconSet::Small, QIconSet::Normal);
	viconset.setPixmap(vpixmap, QIconSet::Small, QIconSet::Active);
	viconset.setPixmap(vbitmap, QIconSet::Small, QIconSet::Disabled);
	viconset.setPixmap(vpixmap, QIconSet::Large, QIconSet::Normal);
	viconset.setPixmap(vpixmap, QIconSet::Large, QIconSet::Active);
	viconset.setPixmap(vbitmap, QIconSet::Large, QIconSet::Disabled);
	settings.writeEntry("/variant/image/iconset", viconset);

	// Point
	QPoint vpoint(424, 242);
	settings.writeEntry("/variant/dimensions/point", vpoint);

	// Int
	int vint = -4242;
	settings.writeEntry("/variant/primitive/int", vint);

	// UInt
	int vuint = 4242;
	settings.writeEntry("/variant/primitive/uint", vuint);

	// Bool
	bool vbool = TRUE;
	settings.writeEntry("/variant/primitive/bool", QVariant(vbool, 0));

	// Double
	double vdouble = 4242.666;
	settings.writeEntry("/variant/primitive/double", vdouble);

	// CString
	QCString vcstring = "Fear the k0w";
	settings.writeEntry("/variant/string/cstring", vcstring);

	// PointArray
	QPointArray vpa(5);
	vpa[0] = QPoint(1, 2);
	vpa[1] = QPoint(3, 4);
	vpa[2] = QPoint(5, 6);
	vpa[3] = QPoint(7, 8);
	vpa[4] = QPoint(9, 10);
	settings.writeEntry("/variant/dimensions/pointarray", vpa);

	// Region
	QRegion vregion;
	vregion = QRegion(1, 2, 3, 4);
	vregion = vregion.unite(QRegion(5, 6, 7, 8));
	vregion = vregion.unite(QRegion(9, 10, 11, 12));
	settings.writeEntry("/variant/dimensions/region", vregion);

	// Cursor
	QCursor vcursor(Qt::forbiddenCursor);
	settings.writeEntry("/variant/gui/cursor", vcursor);

	// SizePolicy
	QSizePolicy vsp;
	vsp.setHorData(QSizePolicy::MinimumExpanding);
	vsp.setVerData(QSizePolicy::Fixed);
	settings.writeEntry("/variant/gui/sizepolicy", vsp);

	// Map
	QMap<QString,QVariant> vmap;
	vmap["string"] = vstring;
	vmap["stringlist"] = vstrlist;
	vmap["font"] = vfont;
	vmap["image"] = vimage;
	vmap["pixmap"] = vpixmap;
	vmap["bitmap"] = vbitmap;
	vmap["color"] = vcolor;
	vmap["brush"] = vbrush;
	vmap["rect"] = vrect;
	vmap["size"] = vsize;
	vmap["palette"] = vpalette;
	vmap["colorgroup"] = vpalette.inactive();
	vmap["iconset"] = viconset;
	vmap["point"] = vpoint;
	vmap["int"] = vint;
	vmap["uint"] = vuint;
	vmap["bool"] = QVariant(vbool, 0);
	vmap["double"] = vdouble;
	vmap["cstring"] = vcstring;
	vmap["pointarray"] = vpa;
	vmap["region"] = vregion;
	vmap["cursor"] = vcursor;
	vmap["sizepolicy"] = vsp;

	// List
	QValueList<QVariant> vlist;
	vlist.append(vstring);
	vlist.append(vstrlist);
	vlist.append(vfont);
	vlist.append(vimage);
	vlist.append(vpixmap);
	vlist.append(vbitmap);
	vlist.append(vcolor);
	vlist.append(vbrush);
	vlist.append(vrect);
	vlist.append(vsize);
	vlist.append(vpalette);
	vlist.append(vpalette.disabled());
	vlist.append(viconset);
	vlist.append(vpoint);
	vlist.append(vint);
	vlist.append(vuint);
	vlist.append(QVariant(vbool, 0));
	vlist.append(vdouble);
	vlist.append(vcstring);
	vlist.append(vpa);
	vlist.append(vregion);
	vlist.append(vcursor);
	vlist.append(vsp);

	// Map + List
	vmap["list"] = vlist;
	vlist.append(vmap);

	settings.writeEntry("/variant/map", vmap);
	settings.writeEntry("/variant/list", vlist);

	// remove
	settings.removeEntry("/variant/image");
	settings.removeEntry("/variant/painter");
	settings.removeEntry("/variant/list");
	settings.removeEntry("/variant/map");
	settings.removeEntry("/variant/string");

	// read the recently destroyed to see if fallback works
	QVariant v = settings.readEntry("/variant/string/string");

	if (v.isValid() && v.type() == QVariant::String)
	    qDebug("  read /variant/string/string = '%s'", v.toString().latin1());
	else
	    qDebug("   failed to read /variant/string/string");

	settings.write();

	settings3.setWritable(TRUE);
	settings3.writeEntry("/variant/string/string",
			     QVariant(QString("this is a string")));
	settings3.write();

	settings3.readEntry("////////");
	settings3.removeEntry("/variant/");

	qDebug("-- done");
    }

    /*
      {
      QVariant v;
      QSettings settings;

      long starttime;
      struct timeval tv;

      qDebug("== read/insert/overwrite/save simple example");

      settings.setPath(QSettings::Unix, "qtrc");

      v = settings.readEntry("/Global/Qt/CursorFlashTime");

      if (v.isValid() && v.type() == QVariant::UInt) {
      qDebug("   read cursor flash time = %d", v.toUInt());
      } else {
      qDebug("   failed to read cursor flash time");
      }

      gettimeofday(&tv, 0);
      starttime = tv.tv_usec;
      v = settings.readEntry("/Global/Qt/Effects/ComboBox");
      gettimeofday(&tv, 0);
      qDebug("   popup effect search took %ld us", tv.tv_usec - starttime);

      if (v.isValid() && v.type() == QVariant::String) {
      qDebug("   read popup menu effect = '%s'", v.toString().latin1());
      }  else {
      qDebug("   failed to read popup menu effect");
      }

      v = settings.readEntry("/Global/Qt/Foo/Bar/Baz/help");

      if (v.isValid() && v.type() == QVariant::String) {
      qDebug("   read help = '%s'", v.toString().latin1());
      } else {
      qDebug("   failed to read help");
      }

      gettimeofday(&tv, 0);
      starttime = tv.tv_usec;
      v = settings.readEntry("/Global/Qt/Foo/Bar/Baz/sexhelp");
      gettimeofday(&tv, 0);
      qDebug("   sexhelp search took %ld us", tv.tv_usec - starttime);

      if (v.isValid() && v.type() == QVariant::String) {
      qDebug("   read sex help = '%s'", v.toString().latin1());
      } else {
      qDebug("   failed to read sex help");
      }

      // change the string in the variant
      v = QString("i'm a slave to your touch, my response automatic");

      settings.writeEntry("/Global/Qt/Foo/Bar/Baz/sexhelp", v);
      v = settings.readEntry("/Global/Qt/Foo/Bar/Baz/sexhelp");

      if (v.isValid() && v.type() == QVariant::String) {
      qDebug("   write1 = '%s'", v.toString().latin1());
      } else {
      qDebug("   failed to read write1");
      }

      settings.setPath(QSettings::Unix, "qtrc.new");
      settings.setWritable(TRUE);

      v = QString("i'm a slave to your touch, my response automatic");

      settings.writeEntry("/Global/Qt/Baz/Bar/Foo/sexhelp", v);
      v = settings.readEntry("/Global/Qt/Baz/Bar/Foo/sexhelp");

      if (v.isValid() && v.type() == QVariant::String) {
      qDebug("   write2 = '%s'", v.toString().latin1());
      } else {
      qDebug("   failed to read write2");
      }

      v = QString("i wish i was a hunter, in search of different food");

      gettimeofday(&tv, 0);
      starttime = tv.tv_usec;
      settings.writeEntry("/MQ3/The/Big/One/WHOO", v);
      gettimeofday(&tv, 0);
      qDebug("   write3 took %ld us", tv.tv_usec - starttime);

      v = settings.readEntry("/MQ3/The/Big/One/WHOO");

      if (v.isValid() && v.type() == QVariant::String) {
      qDebug("   write3 = '%s'", v.toString().latin1());
      } else {
      qDebug("   failed to read write3");
      }

      v = settings.readEntry("/Global/Qt/GlobalStrut");

      if (v.isValid() && v.type() == QVariant::Size) {
      qDebug("   size1 = { %d, %d }", v.toSize().width(), v.toSize().height());
      } else {
      qDebug("   failed to read size1");
      }

      v = QSize(3, 3);
      settings.writeEntry("/Global/Qt/GlobalStrut", v);

      v = settings.readEntry("/Global/Qt/GlobalStrut");

      if (v.isValid() && v.type() == QVariant::Size) {
      qDebug("   size2 = { %d, %d }", v.toSize().width(), v.toSize().height());
      } else {
      qDebug("   failed to read size2");
      }

      gettimeofday(&tv, 0);
      starttime = tv.tv_usec;
      v = settings.readEntry("/Global/Qt/Foo/Bar/Baz/gspot");
      gettimeofday(&tv, 0);
      qDebug("   gspot read took %ld us", tv.tv_usec - starttime);

      if (v.isValid() && v.type() == QVariant::Rect) {
      QRect r = v.toRect();
      qDebug("   gspot = { %d, %d, %d, %d }", r.x(), r.y(), r.width(), r.height());
      } else {
      qDebug("   failed to read gspot");
      }

      settings.write();
      }
    */

    /*
      {
      qDebug("== benchmarking dom");

      long starts, startu;
      struct timeval tv;
      gettimeofday(&tv, 0);
      starts = tv.tv_sec;
      startu = tv.tv_usec;

      QFile file("bradrc");
      if (file.open(IO_ReadOnly)) {
      QDomDocument doc;
      doc.setContent(&file);
      file.close();

      QFile file2("bradrcdom");
      if (file2.open(IO_WriteOnly)) {
      QTextStream ts(&file2);
      doc.save(ts, 4);
      file2.close();
      }
      }

      gettimeofday(&tv, 0);
      qDebug("-- dom took %ld s %ld us", tv.tv_sec - starts, tv.tv_usec - startu);
      }
    */

    /*
      {
      qDebug("== benchmarking qsettings_unix");
      long starts0, startu0;
      struct timeval tv0;
      gettimeofday(&tv0, 0);
      starts0 = tv0.tv_sec;
      startu0 = tv0.tv_usec;

      QSettings settings;

      settings.setWritable(true);
      settings.setPath(QSettings::Unix, "bradrc");

      QString path;

      qDebug("   building large settings tree");
      for (int i = 0; i < 10; i++) {
      path.sprintf("/bradley%d", i);
      settings.writeEntry(path, QVariant(QPalette()));

      for (int ii = 0; ii < 10; ii++) {
      path.sprintf("/bradley%d/tyson%d", i, ii);
      settings.writeEntry(path, QVariant(path));

      for (int iii = 0; iii < 10; iii++ ) {
      path.sprintf("/bradley%d/tyson%d/hughes%d", i, ii, iii);
      settings.writeEntry(path, QVariant(path));

      for (int iv = 0; iv < 10; iv++) {
      path.sprintf("/bradley%d/tyson%d/hughes%d/nyztihke%d",
      i, ii, iii, iv);
      settings.writeEntry(path, QVariant(path));

      for (int v = 0; v < 10; v++) {
      path.sprintf("/bradley%d/tyson%d/hughes%d/nyztihke%d/"
      "reticent%d", i, ii, iii, iv, v);
      settings.writeEntry(path, QVariant(path));
      }
      }
      }
      }
      }

      QVariant var1;

      long starts, startu;
      struct timeval tv;
      gettimeofday(&tv, 0);
      starts = tv.tv_sec;
      startu = tv.tv_usec;
      var1 = settings.readEntry("/foo");
      gettimeofday(&tv, 0);
      qDebug("   worst case search took %ld s %ld us",
      tv.tv_sec - starts, tv.tv_usec - startu);

      if (var1.isValid() && var1.type() == QVariant::String) {
      qDebug("   search succeeded, found '%s'",
      (const char *) var1.toString().utf8());
      } else {
      qDebug("   searched failed");
      }

      qDebug("  writing...");
      gettimeofday(&tv0, 0);
      long startws = tv.tv_sec, startwu = tv.tv_usec;
      settings.setPath(QSettings::Unix, "bradrc2");
      settings.write();
      gettimeofday(&tv0, 0);
      qDebug("  write took %ld s %ld us",
      tv0.tv_sec - startws, tv0.tv_usec - startwu);

      gettimeofday(&tv0, 0);
      qDebug("-- bradrc took %ld s %ld us",
      tv0.tv_sec - starts0, tv0.tv_usec - startu0);
      }
    */

    /*
      // base64 stuffs
      qDebug("== testing base64 encoding");

      // load the Qt logo
      QImage img;
      img.load("qtlogo.png");

      QByteArray ina;
      ina.duplicate((const char *) img.bits(),
      img.width() * img.height() * (img.depth() / 8));

      // time trial
      long starts, startu;
      struct timeval tv;
      gettimeofday(&tv, 0);
      starts = tv.tv_sec;
      startu = tv.tv_usec;
      QByteArray outa = encodeBase64(ina);
      QByteArray ba = decodeBase64(outa);
      gettimeofday(&tv, 0);
      qDebug("-- encode & decode took %ld s %ld us",
      tv.tv_sec - starts, tv.tv_usec - startu);

      // save the base64 output
      QFile file;
      file.setName("qtlogo.base64");
      file.open(IO_WriteOnly);
      file.writeBlock(outa, outa.count());
      file.close();

      // resave the png file
      QImage bimg((uchar *) ba.data(), img.width(), img.height(), img.depth(),
      0, 0, QImage::systemByteOrder());
      bimg.save("qtlogo2.png", "PNG");
    */

    return 0;
}

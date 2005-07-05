/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdir.h"
#include "qscreen_qws.h" //so we can check for rotation
#include "qlibraryinfo.h"

static void addFont(QFontDatabasePrivate *db, const char *family, int weight, bool italic, int pixelSize, const char *file)
{
    QString familyname = QString::fromUtf8(family);
    QString foundryname = "";
    QtFontStyle::Key styleKey;
    styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
    styleKey.weight = weight;
    QtFontFamily *f = privateDb()->family(familyname, true);
    //### get lang info from freetype
    for (int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws)
        f->writingSystems[ws] = QtFontFamily::Supported;
    QtFontFoundry *foundry = f->foundry(foundryname, true);
    QtFontStyle *style = foundry->style(styleKey,  true);
    style->smoothScalable = (pixelSize == 0);
    QtFontSize *size = style->pixelSize(pixelSize?pixelSize:SMOOTH_SCALABLE, true);
    size->fileName = file;
}


/*!
    \internal
*/
static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if (!db || db->count)
        return;
    // initialize Freetype
    FT_Error err;
    err = FT_Init_FreeType(&QFontEngineFT::ft_library);
    Q_ASSERT(!err);

    // Load in font definition file
    QString fn;
#ifndef QT_NO_LIBRARY
    fn = QLibraryInfo::location(QLibraryInfo::PrefixPath);
#endif    
    fn += "/lib/fonts/fontdir";
    FILE* fontdef=fopen(fn.toLocal8Bit().constData(),"r");
    if(!fontdef) {
        qWarning("Cannot find font definition file %s - is Qt installed correctly?",
               fn.toLocal8Bit().constData());
        exit(1);
        //return;
    }
    char buf[200]="";
    char name[200]="";
    char render[200]="";
    char file[200]="";
    char isitalic[10]="";
    do {
        fgets(buf,200,fontdef);
        if (buf[0] != '#') {
            int weight=50;
            int size=0;
            sscanf(buf,"%s %s %s %s %d %d",name,file,render,isitalic,&weight,&size);
            QString filename;
            if (file[0] != '/') {
#ifndef QT_NO_LIBRARY
                filename = QLibraryInfo::location(QLibraryInfo::PrefixPath);
#endif
                filename += "/lib/fonts/";
            }
            filename += file;
            bool italic = isitalic[0] == 'y';
            if (QFile::exists(filename))
                addFont(db, name, weight, italic, size/10, file);
        }
    } while (!feof(fontdef));
    fclose(fontdef);


    QString fontpath;
#ifndef QT_NO_LIBRARY
    fontpath = QLibraryInfo::location(QLibraryInfo::PrefixPath);
#endif
    fontpath += "/lib/fonts";
    QDir dir(fontpath,"*.qpf");
    for (int i=0; i<(int)dir.count(); i++) {
        int u0 = dir[i].indexOf('_');
        int u1 = dir[i].indexOf('_',u0+1);
        int u2 = dir[i].indexOf('_',u1+1);
        int u3 = dir[i].indexOf('.',u1+1);
        if (u2 < 0) u2 = u3;

#if 1
        /*
          Skip fonts for other screen orientations. Such fonts may be
          installed even on a production device. Different orientations
          could have different fonts.
        */
        //### This code could be prettier
        QString rotation;
        if (u2 != u3)
            QString rotation = dir[i].mid(u2+1,u3-u2-1);

        QString screenr;
        if (qt_screen->isTransformed()) {
            screenr = "t";
            QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
            QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
            screenr += QString::number(a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()));
        }

        if (rotation != screenr)
            continue;
#endif

        QString familyname = dir[i].left(u0);
        int pointSize = dir[i].mid(u0+1,u1-u0-1).toInt()/10;
        bool italic = dir[i].mid(u2-1,1) == "i";
        int weight = dir[i].mid(u1+1,u2-u1-1-(italic?1:0)).toInt();
        QtFontFamily *f = db->family(familyname, true);
        for (int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws) {
            if (!requiresOpenType(ws))
                f->writingSystems[ws] = QtFontFamily::Supported;
        }
        QtFontFoundry *foundry = f->foundry("qt", true);
        QtFontStyle::Key styleKey;
        styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
        styleKey.weight = weight;
        QtFontStyle *style = foundry->style(styleKey,  true);
        style->smoothScalable = false;
        style->pixelSize(pointSize, true);
    }

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for (int f = 0; f < db->count; f++) {
        QtFontFamily *family = db->families[f];
        FD_DEBUG("'%s' %s", family->name.latin1(), (family->fixedPitch ? "fixed" : ""));
        for (int i = 0; i < QFont::LastPrivateScript; ++i) {
            FD_DEBUG("\t%s: %s", QFontDatabase::scriptName((QFont::Script) i).latin1(),
                     ((family->scripts[i] & QtFontFamily::Supported) ? "Supported" :
                      (family->scripts[i] & QtFontFamily::UnSupported) == QtFontFamily::UnSupported ?
                      "UnSupported" : "Unknown"));
        }

        for (int fd = 0; fd < family->count; fd++) {
            QtFontFoundry *foundry = family->foundries[fd];
            FD_DEBUG("\t\t'%s'", foundry->name.latin1());
            for (int s = 0; s < foundry->count; s++) {
                QtFontStyle *style = foundry->styles[s];
                FD_DEBUG("\t\t\tstyle: style=%d weight=%d\n"
                         "\t\t\tstretch=%d",
                         style->key.style, style->key.weight,
                         style->key.stretch);
                if (style->smoothScalable)
                    FD_DEBUG("\t\t\t\tsmooth scalable");
                else if (style->bitmapScalable)
                    FD_DEBUG("\t\t\t\tbitmap scalable");
                if (style->pixelSizes) {
                    FD_DEBUG("\t\t\t\t%d pixel sizes",  style->count);
                    for (int z = 0; z < style->count; ++z) {
                        QtFontSize *size = style->pixelSizes + z;
                        FD_DEBUG("\t\t\t\t  size %5d",
                                  size->pixelSize);
                    }
                }
            }
        }
    }
#endif // QFONTDATABASE_DEBUG


}

static inline void load(const QString & = QString(), int = -1)
{
}



static
QFontEngine *loadEngine(int script, const QFontPrivate *fp,
                         const QFontDef &request,
                         QtFontFamily *family, QtFontFoundry *foundry,
                         QtFontStyle *style, QtFontSize *size)
{
    Q_UNUSED(script);
    Q_UNUSED(fp);

    Q_ASSERT(size);


    int pixelSize = size->pixelSize;
    if (!pixelSize || style->smoothScalable && pixelSize == SMOOTH_SCALABLE)
        pixelSize = request.pixelSize;

    if ( foundry->name != QLatin1String("qt") ) { ///#### is this the best way????

    FT_Face face;

    QString file;
#ifndef QT_NO_LIBRARY
    QLibraryInfo::location(QLibraryInfo::PrefixPath);
#endif
    file += "/lib/fonts/";
    file += size->fileName;
    FT_Error err = FT_New_Face(QFontEngineFT::ft_library, file.toLocal8Bit().constData(), 0, &face);
    if (err) {
        FM_DEBUG("loading font file %s failed, err=%x", file.toLocal8Bit().constData(), err);
        Q_ASSERT(!err);
    }
    FT_Set_Pixel_Sizes(face, pixelSize, pixelSize);
    FD_DEBUG("setting pixel size to %d", pixelSize);

    QFontEngine *fe = new QFontEngineFT(request, face);
    return fe;
    } else {
        QString fn;
#ifndef QT_NO_LIBRARY
	fn = QLibraryInfo::location(QLibraryInfo::PrefixPath);
#endif
        fn += QLatin1String("/lib/fonts/")
	    + family->name.toLower()
	    + "_" + QString::number(pixelSize*10)
	    + "_" + QString::number(style->key.weight)
	    + (style->key.style == QFont::StyleItalic ? "i.qpf" : ".qpf");
        //###rotation ###

        QFontEngine *fe = new QFontEngineQPF(request, fn);
        return fe;
    }
}

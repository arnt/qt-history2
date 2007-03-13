/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdir.h"
#include "qscreen_qws.h" //so we can check for rotation
#include "qwindowsystem_qws.h"
#include "qlibraryinfo.h"
#include "qabstractfileengine.h"
#if !defined(QT_NO_FREETYPE)
#include "qfontengine_ft_p.h"
#endif
#include "qfontengine_qpf_p.h"
#include "private/qfactoryloader_p.h"
#include "qabstractfontengine_qws.h"
#include "qabstractfontengine_p.h"
#include <qdatetime.h>

// for mmap
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QFontEngineFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/fontengines"), Qt::CaseInsensitive))
#endif

const quint8 DatabaseVersion = 3;

void QFontDatabasePrivate::addFont(const QString &familyname, const char *foundryname, int weight, bool italic, int pixelSize,
                                   const QByteArray &file, int fileIndex, bool antialiased,
                                   QFontDatabase::WritingSystem primaryWritingSystem)
{
//    qDebug() << "Adding font" << familyname << weight << italic << pixelSize << file << fileIndex << antialiased;
    QtFontStyle::Key styleKey;
    styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
    styleKey.weight = weight;
    styleKey.stretch = 100;
    QtFontFamily *f = family(familyname, true);
    for (int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws) {
        if (primaryWritingSystem == QFontDatabase::Any || primaryWritingSystem == ws)
            f->writingSystems[ws] = QtFontFamily::Supported;
        else
            f->writingSystems[ws] = QtFontFamily::Unsupported;
    }

    QtFontFoundry *foundry = f->foundry(QString::fromLatin1(foundryname), true);
    QtFontStyle *style = foundry->style(styleKey,  true);
    style->smoothScalable = (pixelSize == 0);
    style->antialiased = antialiased;
    QtFontSize *size = style->pixelSize(pixelSize?pixelSize:SMOOTH_SCALABLE, true);
    size->fileName = file;
    size->fileIndex = fileIndex;

    if (stream)
        *stream << familyname << foundry->name << weight << quint8(italic) << pixelSize
                << file << fileIndex << quint8(antialiased) << quint8(primaryWritingSystem);
}

#ifndef QT_NO_QWS_QPF2
void QFontDatabasePrivate::addQPF2File(const QByteArray &file)
{
    struct stat st;
    if (stat(file.constData(), &st))
        return;
    int f = ::open(QFile::encodeName(file), O_RDONLY);
    if (f < 0)
        return;
    const uchar *data = (const uchar *)mmap(0, st.st_size, PROT_READ, MAP_SHARED, f, 0);
    if (data && data != (const uchar *)MAP_FAILED) {
        if (QFontEngineQPF::verifyHeader(data, st.st_size)) {
            QString fontName = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_FontName).toString();
            int pixelSize = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_PixelSize).toInt();
            QVariant weight = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_Weight);
            QVariant style = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_Style);

            if (!fontName.isEmpty() && pixelSize) {
                int fontWeight = 50;
                if (weight.type() == QVariant::Int)
                    fontWeight = weight.toInt();

                bool italic = static_cast<QFont::Style>(style.toInt()) & QFont::StyleItalic;

                addFont(fontName, /*foundry*/ "prerendered", fontWeight, italic,
                        pixelSize, QFile::encodeName(file), /*fileIndex*/ 0,
                        /*antialiased*/ true);
            }
        } else {
            qDebug() << "header verification of QPF2 font" << file << "failed. maybe it is corrupt?";
        }
        munmap((void *)data, st.st_size);
    }
    ::close(f);
}
#endif

#ifndef QT_NO_FREETYPE
QStringList QFontDatabasePrivate::addTTFile(const QByteArray &file, const QByteArray &fontData)
{
    QStringList families;
    extern FT_Library qt_getFreetype();
    FT_Library library = qt_getFreetype();

    int index = 0;
    int numFaces = 0;
    do {
        FT_Face face;
        FT_Error error;
        if (!fontData.isEmpty()) {
            error = FT_New_Memory_Face(library, (const FT_Byte *)fontData.constData(), fontData.size(), index, &face);
        } else {
            error = FT_New_Face(library, file, index, &face);
        }
        if (error != FT_Err_Ok) {
            qDebug() << "FT_New_Face failed with index" << index << ":" << hex << error;
            break;
        }
        numFaces = face->num_faces;

        int weight = QFont::Normal;
        bool italic = face->style_flags & FT_STYLE_FLAG_ITALIC;

        if (face->style_flags & FT_STYLE_FLAG_BOLD)
            weight = QFont::Bold;

        QFontDatabase::WritingSystem primaryWritingSystem = QFontDatabase::Any; // ### get info from freetype
        // detect symbol fonts
        for (int i = 0; i < face->num_charmaps; ++i) {
            FT_CharMap cm = face->charmaps[i];
            if (cm->encoding == ft_encoding_adobe_custom
                    || cm->encoding == ft_encoding_symbol) {
                primaryWritingSystem = QFontDatabase::Symbol;
                break;
            }
        }

        QString family = QString::fromAscii(face->family_name);
        families.append(family);
        addFont(family, /*foundry*/ "", weight, italic,
                /*pixelsize*/ 0, file, index, /*antialias*/ true, primaryWritingSystem);

        FT_Done_Face(face);
        ++index;
    } while (index < numFaces);
    return families;
}
#endif

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt);

extern QString qws_fontCacheDir();

void QFontDatabasePrivate::loadFromCache(const QString &fontPath)
{
    QString fontDirFile = fontPath + QLatin1String("/fontdir");

    QFile binaryDb(qws_fontCacheDir() + QLatin1String("/fontdb"));

    if (!binaryDb.open(QIODevice::ReadOnly))
        qFatal("QFontDatabase::loadFromCache: Could not open font database cache!");

    QDataStream stream(&binaryDb);
    quint8 version = 0;
    quint8 dataStreamVersion = 0;
    stream >> version >> dataStreamVersion;
    if (version != DatabaseVersion || dataStreamVersion != stream.version())
        qFatal("QFontDatabase::loadFromCache: Wrong version of the font database cache detected. Found %d/%d expected %d/%d",
               version, dataStreamVersion, DatabaseVersion, stream.version());

    //qDebug() << "populating database from" << binaryDb.fileName();
    while (!stream.atEnd()) {
        QString familyname, foundryname;
        int weight;
        quint8 italic;
        int pixelSize;
        QByteArray file;
        int fileIndex;
        quint8 antialiased;
        quint8 primaryWritingSystem;
        stream >> familyname >> foundryname >> weight >> italic >> pixelSize
               >> file >> fileIndex >> antialiased >> primaryWritingSystem;
        addFont(familyname, foundryname.toLatin1().constData(), weight, italic, pixelSize, file, fileIndex, antialiased,
                static_cast<QFontDatabase::WritingSystem>(primaryWritingSystem));
    }
}

/*!
    \internal
*/
static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if (!db || db->count)
        return;

    if (db->reregisterAppFonts) {
        db->reregisterAppFonts = false;
        for (int i = 0; i < db->applicationFonts.count(); ++i)
            if (!db->applicationFonts.at(i).families.isEmpty()) {
                registerFont(&db->applicationFonts[i]);
            }
    }

    QString fontpath;
#ifndef QT_NO_SETTINGS
    fontpath = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    fontpath += QLatin1String("/fonts");
#else
    fontpath += QLatin1String("/lib/fonts");
#endif
    QString fontDirFile = fontpath + QLatin1String("/fontdir");

    if(!QFile::exists(fontpath)) {
        qFatal("QFontDatabase: Cannot find font directory %s - is Qt installed correctly?",
               fontpath.toLocal8Bit().constData());
    }

    if (!QWSServer::instance()) {
        db->loadFromCache(fontpath);
        return;
    }

    QString dbFileName = qws_fontCacheDir() + QLatin1String("/fontdb");

    QFile binaryDb(dbFileName + ".tmp");
    binaryDb.open(QIODevice::WriteOnly | QIODevice::Truncate);
    db->stream = new QDataStream(&binaryDb);
    *db->stream << DatabaseVersion << quint8(db->stream->version());
//    qDebug() << "creating binary database at" << binaryDb.fileName();

    // Load in font definition file
    FILE* fontdef=fopen(fontDirFile.toLocal8Bit().constData(),"r");
    if (fontdef) {
        char buf[200]="";
        char name[200]="";
        char render[200]="";
        char file[200]="";
        char isitalic[10]="";
        char flags[10]="";
        do {
            fgets(buf,200,fontdef);
            if (buf[0] != '#') {
                int weight=50;
                int size=0;
                sscanf(buf,"%s %s %s %s %d %d %s",name,file,render,isitalic,&weight,&size,flags);
                QString filename;
                if (file[0] != '/') {
#ifndef QT_NO_SETTINGS
                    filename = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
                    filename += QLatin1String("/fonts/");
#else
                    filename += QLatin1String("/lib/fonts/");
#endif
                }
                filename += file;
                bool italic = isitalic[0] == 'y';
                bool smooth = QByteArray(flags).contains('s');
                if (file[0] && QFile::exists(filename))
                    db->addFont(QString::fromUtf8(name), /*foundry*/"", weight, italic, size/10, file, /*fileIndex*/ 0, smooth);
            }
        } while (!feof(fontdef));
        fclose(fontdef);
    }


    QDir dir(fontpath, QLatin1String("*.qpf"));
    for (int i=0; i<int(dir.count()); i++) {
        int u0 = dir[i].indexOf(QLatin1Char('_'));
        int u1 = dir[i].indexOf(QLatin1Char('_'), u0+1);
        int u2 = dir[i].indexOf(QLatin1Char('_'), u1+1);
        int u3 = dir[i].indexOf(QLatin1Char('.'), u1+1);
        if (u2 < 0) u2 = u3;

        QString familyname = dir[i].left(u0);
        int pixelSize = dir[i].mid(u0+1,u1-u0-1).toInt()/10;
        bool italic = dir[i].mid(u2-1,1) == QLatin1String("i");
        int weight = dir[i].mid(u1+1,u2-u1-1-(italic?1:0)).toInt();

        db->addFont(familyname, /*foundry*/ "qt", weight, italic, pixelSize, QFile::encodeName(dir.absoluteFilePath(dir[i])),
                    /*fileIndex*/ 0, /*antialiased*/ true);
    }

#ifndef QT_NO_FREETYPE
    dir.setNameFilters(QStringList() << QLatin1String("*.ttf")
                       << QLatin1String("*.ttc") << QLatin1String("*.pfa")
                       << QLatin1String("*.pfb"));
    dir.refresh();
    for (int i = 0; i < int(dir.count()); ++i) {
        const QByteArray file = QFile::encodeName(dir.absoluteFilePath(dir[i]));
//        qDebug() << "looking at" << file;
        db->addTTFile(file);
    }
#endif
#ifndef QT_NO_QWS_QPF2
    dir.setNameFilters(QStringList() << QLatin1String("*.qpf2"));
    dir.refresh();
    for (int i = 0; i < int(dir.count()); ++i) {
        const QByteArray file = QFile::encodeName(dir.absoluteFilePath(dir[i]));
//        qDebug() << "looking at" << file;
        db->addQPF2File(file);
    }
#endif

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for (int f = 0; f < db->count; f++) {
        QtFontFamily *family = db->families[f];
        FD_DEBUG("'%s' %s", qPrintable(family->name), (family->fixedPitch ? "fixed" : ""));
#if 0
        for (int i = 0; i < QFont::LastPrivateScript; ++i) {
            FD_DEBUG("\t%s: %s", qPrintable(QFontDatabase::scriptName((QFont::Script) i)),
                     ((family->scripts[i] & QtFontFamily::Supported) ? "Supported" :
                      (family->scripts[i] & QtFontFamily::UnSupported) == QtFontFamily::UnSupported ?
                      "UnSupported" : "Unknown"));
        }
#endif

        for (int fd = 0; fd < family->count; fd++) {
            QtFontFoundry *foundry = family->foundries[fd];
            FD_DEBUG("\t\t'%s'", qPrintable(foundry->name));
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

#ifndef QT_NO_LIBRARY
    QStringList pluginFoundries = loader()->keys();
//    qDebug() << "plugin foundries:" << pluginFoundries;
    for (int i = 0; i < pluginFoundries.count(); ++i) {
        const QString foundry(pluginFoundries.at(i));

        QFontEngineFactoryInterface *factory = qobject_cast<QFontEngineFactoryInterface *>(loader()->instance(foundry));
        if (!factory) {
            qDebug() << "Could not load plugin for foundry" << foundry;
            continue;
        }

        QList<QCustomFontInfo> fonts = factory->availableFonts();
        for (int i = 0; i < fonts.count(); ++i) {
            QCustomFontInfo info = fonts.at(i);

            int weight = info.weight();
            if (weight <= 0)
                weight = QFont::Normal;

            db->addFont(info.family(), foundry.toLatin1().constData(),
                        weight, info.style() != QFont::Normal,
                        qRound(info.pixelSize()), /*file*/QByteArray(),
                        /*fileIndex*/0, /*antiAliased*/true);
        }
    }
#endif

    delete db->stream;
    db->stream = 0;
    QFile::remove(dbFileName);
    binaryDb.rename(dbFileName);
}

static inline void load(const QString & = QString(), int = -1)
{
    initializeDb();
}

#ifndef QT_NO_FREETYPE

#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
#define X_SIZE(face,i) ((face)->available_sizes[i].x_ppem)
#define Y_SIZE(face,i) ((face)->available_sizes[i].y_ppem)
#else
#define X_SIZE(face,i) ((face)->available_sizes[i].width << 6)
#define Y_SIZE(face,i) ((face)->available_sizes[i].height << 6)
#endif

#endif // QT_NO_FREETYPE

static
QFontEngine *loadEngine(int script, const QFontPrivate *fp,
                         const QFontDef &request,
                         QtFontFamily *family, QtFontFoundry *foundry,
                         QtFontStyle *style, QtFontSize *size)
{
    Q_UNUSED(script);
    Q_UNUSED(fp);
#ifdef QT_NO_FREETYPE
    Q_UNUSED(foundry);
#endif
#ifdef QT_NO_QWS_QPF
    Q_UNUSED(family);
#endif
    Q_ASSERT(size);

    int pixelSize = size->pixelSize;
    if (!pixelSize || style->smoothScalable && pixelSize == SMOOTH_SCALABLE)
        pixelSize = request.pixelSize;

#ifndef QT_NO_QPF2
    if (foundry->name == QLatin1String("prerendered")) {
        int f = ::open(QFile::encodeName(size->fileName), O_RDONLY);
        if (f >= 0) {
            QFontEngineQPF *fe = new QFontEngineQPF(request, f);
            if (fe->isValid())
                return fe;
            qDebug() << "fontengine is not valid!";
            delete fe; // will close f
        }
    } else
#endif
#ifndef QT_NO_FREETYPE
    if ( foundry->name != QLatin1String("qt") ) { ///#### is this the best way????
        QString file = size->fileName;

        QFontDef def = request;
        def.pixelSize = pixelSize;

        static bool dontShareFonts = !qgetenv("QWS_NO_SHARE_FONTS").isEmpty();
        bool shareFonts = !dontShareFonts;

        QFontEngine *engine = 0;

        if (file.isEmpty()) {
            QFontEngineFactoryInterface *factory = qobject_cast<QFontEngineFactoryInterface *>(loader()->instance(foundry->name));
            if (factory) {
                QCustomFontInfo info;
                info.setFamily(request.family);
                info.setPixelSize(request.pixelSize);
                info.setStyle(request.style);
                info.setWeight(request.weight);
                // #### antialiased

                QAbstractFontEngine *customEngine = factory->create(info);
                if (customEngine) {
                    engine = new QProxyFontEngine(customEngine, def);

                    if (shareFonts) {
                        QVariant hint = customEngine->fontProperty(QAbstractFontEngine::GlyphShareHint);
                        if (hint.isValid())
                            shareFonts = hint.toBool();
                        else
                            shareFonts = (pixelSize < 64);
                    }
                }
            }
        } else if (QFile::exists(file) || privateDb()->isApplicationFont(file)) {
            QFontEngine::FaceId faceId;
            faceId.filename = file.toLocal8Bit();
            faceId.index = size->fileIndex;

            QFontEngineFT *fte = new QFontEngineFT(def);
            if (fte->init(faceId, style->antialiased)) {
#ifdef QT_NO_QPF2
                return fte;
#else
                engine = fte;
                // try to distinguish between bdf and ttf fonts we can pre-render
                // and don't try to share outline fonts
                shareFonts = shareFonts
                             && !fte->drawAsOutline()
                             && !fte->getSfntTable(MAKE_TAG('h', 'e', 'a', 'd')).isEmpty();
#endif
            } else {
                delete fte;
            }
        }
        if (engine) {
            if (shareFonts) {
                QFontEngineQPF *fe = new QFontEngineQPF(def, -1, engine);
                if (fe->isValid())
                    return fe;
                engine = 0;
            } else {
                return engine;
            }
        }
    } else
#endif // QT_NO_FREETYPE
    {
#ifndef QT_NO_QWS_QPF
        QString fn;
#ifndef QT_NO_SETTINGS
	fn = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
        fn += QLatin1String("/fonts/");
#else
        fn += QLatin1String("/lib/fonts/");
#endif
        fn += family->name.toLower()
              + QLatin1String("_") + QString::number(pixelSize*10)
              + QLatin1String("_") + QString::number(style->key.weight)
              + (style->key.style == QFont::StyleItalic ?
                 QLatin1String("i.qpf") : QLatin1String(".qpf"));
        //###rotation ###

        QFontEngine *fe = new QFontEngineQPF1(request, fn);
        return fe;
#endif // QT_NO_QWS_QPF
    }
    return new QFontEngineBox(pixelSize);
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
    QFontDatabasePrivate *db = privateDb();
    fnt->families = db->addTTFile(QFile::encodeName(fnt->fileName), fnt->data);
    db->reregisterAppFonts = true;
}

bool QFontDatabase::removeApplicationFont(int handle)
{
    QFontDatabasePrivate *db = privateDb();
    if (handle < 0 || handle >= db->applicationFonts.count())
        return false;

    db->applicationFonts[handle] = QFontDatabasePrivate::ApplicationFont();

    db->reregisterAppFonts = true;
    db->invalidate();
    return true;
}

bool QFontDatabase::removeAllApplicationFonts()
{
    QFontDatabasePrivate *db = privateDb();
    if (db->applicationFonts.isEmpty())
        return false;

    db->applicationFonts.clear();
    db->invalidate();
    return true;
}


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

// the miscrosoft platform SDK says that the Unicode versions of
// TextOut and GetTextExtentsPoint32 are supported on all platforms, so we use them
// exclusively to simplify code, save a lot of conversions into the local encoding
// and have generally better unicode support :)

#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qtextengine_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"

#include "qwidget.h"
#include "qpainter.h"
#include <limits.h>
#include "qt_windows.h"
#include <private/qapplication_p.h>
#include "qapplication.h"
#include <private/qunicodetables_p.h>
#include <qfontdatabase.h>


extern HDC   shared_dc;                // common dc for all fonts


// ### maybe move to qapplication_win
QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool /*scale*/)
{
    QString family = QT_WA_INLINE(QString::fromUtf16((ushort*)lf.lfFaceName),
                                   QString::fromLocal8Bit((char*)lf.lfFaceName));
    QFont qf(family);
    qf.setItalic(lf.lfItalic);
    if (lf.lfWeight != FW_DONTCARE) {
        int weight;
        if (lf.lfWeight < 400)
            weight = QFont::Light;
        else if (lf.lfWeight < 600)
            weight = QFont::Normal;
        else if (lf.lfWeight < 700)
            weight = QFont::DemiBold;
        else if (lf.lfWeight < 800)
            weight = QFont::Bold;
        else
            weight = QFont::Black;
        qf.setWeight(weight);
    }
    int lfh = qAbs(lf.lfHeight);
    Q_ASSERT(shared_dc);
    qf.setPointSizeF(lfh * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY));
    qf.setUnderline(false);
    qf.setOverline(false);
    qf.setStrikeOut(false);
    return qf;
}


static inline float pixelSize(const QFontDef &request, int dpi)
{
    float pSize;
    if (request.pointSize != -1)
        pSize = request.pointSize * dpi/ 72.;
    else
        pSize = request.pixelSize;
    return pSize;
}

static inline float pointSize(const QFontDef &fd, int dpi)
{
    float pSize;
    if (fd.pointSize < 0)
        pSize = fd.pixelSize * 72. / ((float)dpi);
    else
        pSize = fd.pointSize;
    return pSize;
}

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
    if (QFontCache::instance)
        return;
    shared_dc = CreateCompatibleDC(qt_win_display_dc());
    if (!shared_dc)
        qErrnoWarning("QFont::initialize: CreateCompatibleDC failed");
    new QFontCache();
}

void QFont::cleanup()
{
    delete QFontCache::instance;
    DeleteDC(shared_dc);
    shared_dc = 0;
}

void QFontPrivate::load(int script)
{
    // NOTE: the X11 and Windows implementations of this function are
    // identical... if you change one, change both.

    // sanity checks
    if (!QFontCache::instance)
        qWarning("Must construct a QApplication before a QFont");
    Q_ASSERT(script >= 0 && script < QUnicodeTables::ScriptCount);

    QFontDef req = request;
    int px = qRound(pixelSize(req, dpi));
    req.pixelSize = px;

    // set the point size to 0 to get better caching
    req.pointSize = 0;

    if (! engineData) {
        QFontCache::Key key(req, QUnicodeTables::Common);

        // look for the requested font in the engine data cache
        engineData = QFontCache::instance->findEngineData(key);

        if (! engineData) {
            // create a new one
            engineData = new QFontEngineData;
            QFontCache::instance->insertEngineData(key, engineData);
        } else {
            engineData->ref.ref();
        }
    }

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = qRound(pointSize(request, dpi));

    // the cached engineData could have already loaded the engine we want
    if (engineData->engines[script]) return;

    // load the font
    QFontEngine *engine = 0;
    //    double scale = 1.0; // ### TODO: fix the scale calculations

    // list of families to try
    QStringList family_list;

    if (!req.family.isEmpty()) {
        family_list = req.family.split(',');

        // append the substitute list for each family in family_list
        QStringList subs_list;
        QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
        for (; it != end; ++it)
            subs_list += QFont::substitutes(*it);
        family_list += subs_list;

        if(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based && req.family.toLower() == "ms sans serif") {
            // small hack for Dos based machines to get the right font for non
            // latin text when using the default font.
            family_list << "Arial" << "Tahoma" << "Verdana";
        }
        // append the default fallback font for the specified script
        // family_list << ... ; ###########

        // add the default family
        QString defaultFamily = QApplication::font().family();
        if (! family_list.contains(defaultFamily))
            family_list << defaultFamily;

        // add QFont::defaultFamily() to the list, for compatibility with
        // previous versions
        family_list << QApplication::font().defaultFamily();
    }

    // null family means find the first font matching the specified script
    family_list << QString();

    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
    for (; ! engine && it != end; ++it) {
        req.family = *it;

        engine = QFontDatabase::findFont(script, this, req);
        if (engine) {
            if (engine->type() != QFontEngine::Box)
                break;

            if (! req.family.isEmpty())
                engine = 0;

            continue;
        }
    }

    engine->ref.ref();
    engineData->engines[script] = engine;
}

HFONT QFont::handle() const
{
    QFontEngine *engine = d->engineForScript(QUnicodeTables::Common);
    return engine->hfont;
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName(const QString &name)
{
    setFamily(name);
}

QString QFont::defaultFamily() const
{
    switch(d->request.styleHint) {
        case QFont::Times:
            return QString::fromLatin1("Times New Roman");
        case QFont::Courier:
            return QString::fromLatin1("Courier New");
        case QFont::Decorative:
            return QString::fromLatin1("Bookman Old Style");
        case QFont::Helvetica:
            return QString::fromLatin1("Arial");
        case QFont::System:
        default:
            return QString::fromLatin1("MS Sans Serif");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("arial");
}

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

#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpaintdevice.h"
#include "qstring.h"
#include <private/qt_mac_p.h>
#include <private/qtextengine_p.h>
#include <private/qunicodetables_p.h>
#include <qapplication.h>
#include "qfontdatabase.h"
#include <qpainter.h>
#include "qtextengine_p.h"
#include <stdlib.h>

QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;

int qt_mac_pixelsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pixelSize == -1)
        ret = def.pointSize *  dpi / 720.;
    else
        ret = def.pixelSize;
    return qRound(ret);
}
int qt_mac_pointsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pointSize == -1)
        ret = def.pixelSize * 720. / float(dpi);
    else
        ret = def.pointSize;
    return qRound(ret);
}

int QFontMetrics::charWidth(const QString &str, int pos) const
{
    if (pos < 0 || pos > str.length())
        return 0;

    const QChar &ch = str.unicode()[pos];

    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    int cwidth;

    if (script >= QFont::Arabic && script <= QFont::Khmer) {
        // complex script shaping. Have to do some hard work
        int from = qMax(0,  pos - 8);
        int to = qMin(str.length(), pos + 8);
        QString cstr(str.unicode() + from, to - from);
        QTextEngine layout(cstr, d);
        layout.setMode(QTextEngine::WidthOnly);
        layout.itemize();
        cwidth = (int)layout.width(pos - from, 1);
    } else if (::category(ch) == QChar::Mark_NonSpacing) {
        cwidth = 0;
    } else {
        cwidth = width(ch);
    }
    return cwidth;
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName(const QString &name)
{
    setFamily(name);
}

void QFont::cleanup()
{
    delete QFontCache::instance;
}

// Returns an ATSFontRef
Qt::HANDLE QFont::handle() const
{
    if (! d->engineData)
        d->load(QFont::NoScript);
    if (d->engineData && d->engineData->engine)
        return (Qt::HANDLE)((QFontEngineMac*)d->engineData->engine)->fontref;
    return 0;
}

void QFontPrivate::load(QFont::Script script)
{
    // sanity checks
    if (!QFontCache::instance)
        qWarning("Must construct a QApplication before a QFont");
    Q_ASSERT(script >= 0 && script < QFont::LastPrivateScript);
    Q_UNUSED(script);

    QFontDef req = request;
    req.pixelSize = qt_mac_pixelsize(request, dpi);

    // set the point size to 0 to get better caching
    req.pointSize = 0;
    QFontCache::Key key = QFontCache::Key(req, QFont::NoScript, screen);

    if(!(engineData = QFontCache::instance->findEngineData(key))) {
        engineData = new QFontEngineData;
        QFontCache::instance->insertEngineData(key, engineData);
    } else {
        ++engineData->ref;
    }
    if(engineData->engine) // already loaded
        return;

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = qRound(qt_mac_pointsize(request, dpi));

    QFontEngineMac *engine = 0;
    if(QFontEngine *e = QFontCache::instance->findEngine(key)) {
        Q_ASSERT(e->type() == QFontEngine::Mac);
        ++e->ref;
        engineData->engine = e;
        engine = static_cast<QFontEngineMac *>(e);
        return; // the font info and fontdef should already be filled
    }

#if !defined(QT_NO_DEBUG)
    if (req.family == QLatin1String("__Qt__Box__Engine__")) {
        QFontEngine *e = new QTestFontEngine(request.pixelSize);
        engineData->engine = e;
        e->fontDef = req;
        QFontCache::instance->insertEngine(key, e);
        return;
    }
#endif

    engine = new QFontEngineMac;
    engineData->engine = engine;
    ++engine->ref; //a ref for the engineData->engine
    if(!engine->fontref) {
        //find the font
        QStringList family_list = request.family.split(',');
        // append the substitute list for each family in family_list
        {
            QStringList subs_list;
            QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
            for (; it != end; ++it)
                subs_list += QFont::substitutes(*it);
            family_list += subs_list;
        }
        // add QFont::defaultFamily() to the list, for compatibility with
        // previous versions
        family_list << QApplication::font().defaultFamily();
        for(QStringList::ConstIterator it = family_list.begin(); it !=  family_list.end(); ++it) {
            if(ATSFontRef fontref = ATSFontFindFromName(QCFString(*it), kATSOptionFlagsDefault)) {
                QCFString actualName;
                if(ATSFontGetName(fontref, kATSOptionFlagsDefault,
                                  &actualName) == noErr) {
                    if(static_cast<QString>(actualName) == (*it)) {
                        engine->fontref = fontref;
                        break;
                    }
                }
                if(!engine->fontref) //just take one if it isn't set yet
                    engine->fontref = fontref;
            }
        }
    }
    { //fill in the engine's font definition
        engine->fontDef = request; //copy..
        if(engine->fontDef.pointSize == -1)
            engine->fontDef.pointSize = qt_mac_pointsize(engine->fontDef, dpi);
        else
            engine->fontDef.pixelSize = qt_mac_pixelsize(engine->fontDef, dpi);
        {
            QCFString actualName;
            Q_ASSERT(engine->type() == QFontEngine::Mac);
            if (ATSFontGetName(engine->fontref, kATSOptionFlagsDefault, &actualName)
                == noErr) {
                engine->fontDef.family = actualName;
            }
        }
    }
    QFontCache::instance->insertEngine(key, engine);
}

void QFont::initialize()
{
    if(!QFontCache::instance)
        new QFontCache();
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
        case QFont::System:
        default:
            return QString::fromLatin1("Helvetica");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("Helvetica");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("Arial");
}

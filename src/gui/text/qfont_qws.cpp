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

#include "qwidget.h"
#include "qpainter.h"
#include "qfont_p.h"
#include <private/qunicodetables_p.h>
#include "qfontdatabase.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qmemorymanager_qws.h"
#include "qtextengine_p.h"
#include "qfontengine_p.h"


void QFont::initialize()
{
    // create global font cache
    if (! QFontCache::instance) (void) new QFontCache;
}

void QFont::cleanup()
{
    // delete the global font cache
    delete QFontCache::instance;
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

Qt::HANDLE QFont::handle() const
{
    QFontEngine *engine = d->engineForScript(QUnicodeTables::Common);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Freetype)
        return static_cast<QFontEngineFT *>(engine)->handle();
    return 0;
}

QString QFont::rawName() const
{
    return "unknown";
}

void QFont::setRawName(const QString &)
{
}

QString QFont::defaultFamily() const
{
    switch(d->request.styleHint) {
        case QFont::Times:
            return QString::fromLatin1("times");
        case QFont::Courier:
            return QString::fromLatin1("courier");
        case QFont::Decorative:
            return QString::fromLatin1("old english");
        case QFont::Helvetica:
        case QFont::System:
        default:
            return QString::fromLatin1("helvetica");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
    qFatal("QFont::lastResortFont: Cannot find any reasonable font");
    // Shut compiler up
    return QString::null;
}

void QFontPrivate::load(int)
{
    QFontDef req = request;
    int script = QUnicodeTables::Common;

    // 75 dpi on embedded
    if (req.pixelSize == -1)
        req.pixelSize = req.pointSize/10;
    if (req.pointSize == -1)
        req.pointSize = req.pixelSize*10;

    if (! engineData) {
        QFontCache::Key key(req, script);

        // look for the requested font in the engine data cache
        engineData = QFontCache::instance->findEngineData(key);

        if (! engineData) {
            // create a new one
            engineData = new QFontEngineData;
            QFontCache::instance->insertEngineData(key, engineData);
        } else {
            ++engineData->ref;
        }
    }

    // the cached engineData could have already loaded the engine we want
    if (engineData->engine) return;

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
    family_list << QString::null;

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

    ++engine->ref;
    engineData->engine = engine;
}

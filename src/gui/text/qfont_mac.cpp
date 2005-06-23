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
#include "qfont_p.h"
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

int qt_mac_pixelsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pixelSize == -1)
        ret = def.pointSize *  dpi / 72.;
    else
        ret = def.pixelSize;
    return qRound(ret);
}
int qt_mac_pointsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pointSize < 0)
        ret = def.pixelSize * 72. / float(dpi);
    else
        ret = def.pointSize;
    return qRound(ret);
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
        d->load(QUnicodeTables::Common);
    if (d->engineData && d->engineData->engine)
        return (Qt::HANDLE)((QFontEngineMac*)d->engineData->engine)->familyref;
    return 0;
}

void QFontPrivate::load(int script)
{
    // sanity checks
    if (!QFontCache::instance)
        qWarning("Must construct a QApplication before a QFont");
    Q_ASSERT(script >= 0 && script < QUnicodeTables::ScriptCount);
    Q_UNUSED(script);

    QFontDef req = request;
    req.pixelSize = qt_mac_pixelsize(request, dpi);

    // set the point size to 0 to get better caching
    req.pointSize = 0;
    QFontCache::Key key = QFontCache::Key(req, QUnicodeTables::Common, screen);

    if(!(engineData = QFontCache::instance->findEngineData(key))) {
        engineData = new QFontEngineData;
        QFontCache::instance->insertEngineData(key, engineData);
    } else {
        engineData->ref.ref();
    }
    if(engineData->engine) // already loaded
        return;

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = qRound(qt_mac_pointsize(request, dpi));

    QFontEngineMac *engine = 0;
    if(QFontEngine *e = QFontCache::instance->findEngine(key)) {
        Q_ASSERT(e->type() == QFontEngine::Mac);
        e->ref.ref();
        engineData->engine = e;
        engine = static_cast<QFontEngineMac *>(e);
        return; // the font info and fontdef should already be filled
    }

    extern bool qt_enable_test_font; // qfontdatabase.cpp
    if (qt_enable_test_font && req.family == QLatin1String("__Qt__Box__Engine__")) {
        QFontEngine *e = new QTestFontEngine(request.pixelSize);
        engineData->engine = e;
        e->fontDef = req;
        e->kerning = kerning;
        QFontCache::instance->insertEngine(key, e);
        return;
    }

    engine = new QFontEngineMac;
    engineData->engine = engine;
    engine->ref.ref(); //a ref for the engineData->engine
    if(!engine->familyref) {
        //find the font
        QStringList family_list = req.family.split(',');
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

        //find it!
        QHash<QString, ATSFontFamilyRef> mac_families;
        {
            ATSFontFamilyIterator iterator;
            if(!ATSFontFamilyIteratorCreate(kATSFontContextGlobal, 0, 0,
                                            kATSOptionFlagsRestrictedScope, &iterator)) {
                for(ATSFontFamilyRef family; ATSFontFamilyIteratorNext(iterator, &family) == noErr; ) {
                    QCFString actualName;
                    if(ATSFontFamilyGetName(family, kATSOptionFlagsDefault, &actualName) == noErr)
                        mac_families.insert(static_cast<QString>(actualName).toLower(), family);
                }
                ATSFontFamilyIteratorRelease(&iterator);
            }
        }
        for(QStringList::ConstIterator it = family_list.begin(); it !=  family_list.end(); ++it) {
            if(mac_families.contains((*it).toLower())) {
                engine->familyref = mac_families.value((*it).toLower());
                break;
            }
            if(ATSFontFamilyRef family = ATSFontFamilyFindFromName(QCFString(*it), kATSOptionFlagsDefault)) {
                QCFString actualName;
                if(ATSFontFamilyGetName(family, kATSOptionFlagsDefault, &actualName) == noErr) {
                    if(static_cast<QString>(actualName) == (*it)) {
                        engine->familyref = family;
                        break;
                    }
                }
                engine->familyref = family; //just take one if it isn't set yet
            }
        }
    }
    { //fill in the engine's font definition
        engine->kerning = kerning;
        engine->fontDef = request; //copy..
        if(engine->fontDef.pointSize < 0)
            engine->fontDef.pointSize = qt_mac_pointsize(engine->fontDef, dpi);
        else
            engine->fontDef.pixelSize = qt_mac_pixelsize(engine->fontDef, dpi);
        {
            QCFString actualName;
            Q_ASSERT(engine->type() == QFontEngine::Mac);
            if (ATSFontFamilyGetName(engine->familyref, kATSOptionFlagsDefault, &actualName) == noErr)
                engine->fontDef.family = actualName;
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
    return QString::fromLatin1("Geneva");
}

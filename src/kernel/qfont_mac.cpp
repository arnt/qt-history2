/****************************************************************************
**
** Implementation of QFont/QFontMetrics class for mac.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstring.h"
#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qpaintdevicemetrics.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qt_mac.h"
#include "qpaintdevice.h"
#include <qdict.h>
#include <qapplication.h>
#include <qpainter.h>
#include <private/qunicodetables_p.h>
#include <stdlib.h>

QString pstring2qstring(const unsigned char *c);
void qstring2pstring(QString s, Str255 str, TextEncoding encoding=0, int len=-1); //qglobal.cpp
CFStringRef qstring2cfstring(const QString &str); //qglobal.cpp
QString cfstring2qstring(CFStringRef str); //qglobal.cpp

QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;

static inline int qt_mac_pixelsize(const QFontDef &def, QPaintDevice *pdev)
{
    float ret;
    if(def.pixelSize == -1) {
	if(pdev) {
	    ret = def.pointSize *  QPaintDeviceMetrics(pdev).logicalDpiY() / 720.;
	} else {
	    short vr, hr;
	    ScreenRes(&hr, &vr);
	    ret = def.pointSize * vr / 720.;
	}
    } else {
	ret = def.pixelSize;
    }
    return (int)(ret + .5);
}
static inline int qt_mac_pointsize(const QFontDef &def, QPaintDevice *pdev)
{
    float ret;
    if(def.pointSize == -1) {
	if(pdev) {
	    ret = def.pixelSize * 720. / QPaintDeviceMetrics(pdev).logicalDpiY();
	} else {
	    short vr, hr;
	    ScreenRes(&hr, &vr);
	    ret = def.pixelSize * 720. / vr;
	}
    } else {
	ret = def.pointSize;
    }
    return (int)(ret + .5);
}

/* Qt platform dependant functions */
int QFontMetrics::width(QChar c) const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    Q_ASSERT( engine != 0 );
    Q_ASSERT( engine->type() == QFontEngine::Mac );
    return ((QFontEngineMac*) engine)->doTextTask(&c, 0, 1, 1,
						  QFontEngineMac::WIDTH);
}

int QFontMetrics::charWidth(const QString &str, int pos) const
{
    QTextEngine layout(str, d);
    layout.itemize(QTextEngine::WidthOnly);
    return layout.width(pos, 1);
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

// Return a font family ID
Qt::HANDLE QFont::handle() const
{
    if ( ! d->engineData )
        d->load( QFont::NoScript );
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

    QFontEngineMac *engine = NULL;

    QFontDef req = request;
    req.pixelSize = qt_mac_pixelsize(request, paintdevice);
    req.pointSize = 0;
    QFontCache::Key key = QFontCache::Key(req, QFont::NoScript, screen);

    if(!(engineData = QFontCache::instance->findEngineData(key))) {
	engineData = new QFontEngineData;
	QFontCache::instance->insertEngineData(key, engineData);
    } else {
	engineData->ref();
    }
    if ( engineData->engine ) // already loaded
	return;

    if(QFontEngine *e = QFontCache::instance->findEngine(key)) {
	Q_ASSERT(e->type() == QFontEngine::Mac);
	e->ref();
	engineData->engine = e;
	engine = (QFontEngineMac*)e;
	return; // the font info and fontdef should already be filled
    }

    engine = new QFontEngineMac;
    engineData->engine = engine;
    if(!engine->fontref) {
	//find the font
	QStringList family_list = QStringList::split( ',', request.family );
	// append the substitute list for each family in family_list
	{
	    QStringList subs_list;
	    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
	    for ( ; it != end; ++it )
		subs_list += QFont::substitutes( *it );
	    family_list += subs_list;
	}
	// add QFont::defaultFamily() to the list, for compatibility with
	// previous versions
	family_list << QApplication::font().defaultFamily();
	for(QStringList::ConstIterator it = family_list.begin(); it !=  family_list.end(); ++it) {
	    if(ATSFontRef fontref = ATSFontFindFromName(qstring2cfstring((*it)), kATSOptionFlagsDefault)) {
		CFStringRef actualName;
		if(ATSFontGetName(fontref, kATSOptionFlagsDefault, &actualName) == noErr) {
		    if(cfstring2qstring(actualName) == (*it)) {
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
	    engine->fontDef.pointSize = qt_mac_pointsize(engine->fontDef, paintdevice);
	else
	    engine->fontDef.pixelSize = qt_mac_pixelsize(engine->fontDef, paintdevice);
	{
	    CFStringRef actualName;
	    Q_ASSERT(engine->type() == QFontEngine::Mac);
	    if(ATSFontGetName(engine->fontref, kATSOptionFlagsDefault, &actualName) == noErr)
		engine->fontDef.family = cfstring2qstring(actualName);
	}
    }
    QFontCache::instance->insertEngine(key, engine);
}

void QFont::initialize()
{
    if(!QFontCache::instance)
        new QFontCache();
}

bool QFont::dirty() const
{
    return d->engineData == 0;
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

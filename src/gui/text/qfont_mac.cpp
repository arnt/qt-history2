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

extern float qt_mac_defaultDpi_x(); //qpaintdevice_mac.cpp

int qt_mac_pixelsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pixelSize == -1)
        ret = def.pointSize *  dpi / qt_mac_defaultDpi_x();
    else
        ret = def.pixelSize;
    return qRound(ret);
}
int qt_mac_pointsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pointSize < 0)
        ret = def.pixelSize * qt_mac_defaultDpi_x() / float(dpi);
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

/*!
  Returns an ATSUFontID
*/
quint32 QFont::macFontID() const
{
#if 1
    QFontEngine *fe = d->engineForScript(QUnicodeTables::Common);
    if (fe && fe->type() == QFontEngine::Mac)
        return static_cast<QFontEngineMacMulti*>(fe)->macFontID();
#else
    Str255 name;
    if(FMGetFontFamilyName((FMFontFamily)((UInt32)handle()), name) == noErr) {
        short fnum;
        GetFNum(name, &fnum);
        return fnum;
    }
#endif
    return 0;
}

// Returns an ATSUFonFamilyRef
Qt::HANDLE QFont::handle() const
{
#if 0
    QFontEngine *fe = d->engineForScript(QUnicodeTables::Common);
    if (fe && fe->type() == QFontEngine::Mac)
        return (Qt::HANDLE)static_cast<QFontEngineMacMulti*>(fe)->fontFamilyRef();
#endif
    return 0;
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

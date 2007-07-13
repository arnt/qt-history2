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
//#include "qmemorymanager_qws.h"
#include "qtextengine_p.h"
#include "qfontengine_p.h"
#if !defined(QT_NO_FREETYPE)
#include "qfontengine_ft_p.h"
#endif


void QFont::initialize()
{ }

void QFont::cleanup()
{
    QFontCache::cleanup();
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

Qt::HANDLE QFont::handle() const
{
#ifndef QT_NO_FREETYPE
    return freetypeFace();
#endif
    return 0;
}

FT_Face QFont::freetypeFace() const
{
#ifndef QT_NO_FREETYPE
    QFontEngine *engine = d->engineForScript(QUnicodeTables::Common);
    if (engine->type() == QFontEngine::Multi)
        engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
    if (engine->type() == QFontEngine::Freetype) {
        const QFontEngineFT *ft = static_cast<const QFontEngineFT *>(engine);
        return ft->non_locked_face();
    }
#endif
    return 0;
}

QString QFont::rawName() const
{
    return QLatin1String("unknown");
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
    return QString();
}


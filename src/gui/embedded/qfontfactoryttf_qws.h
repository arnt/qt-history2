/****************************************************************************
**
** Definition of QFontFactory for Truetype class for Embedded Qt.
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

#ifndef QFONTFACTORYTTF_QWS_H
#define QFONTFACTORYTTF_QWS_H

#ifndef QT_H
#include "qfontmanager_qws.h"
#endif // QT_H

#ifndef QT_NO_FREETYPE

#if 0 //new freetype version
#include <ft2build.h>
#include FT_FREETYPE_H
#else
extern "C" {
#include <freetype/freetype.h>
}
#endif
// ascent, descent, width(ch), width(string), maxwidth?
// leftbearing, rightbearing, minleftbearing,minrightbearing
// leading

class QFontFactoryFT : public QFontFactory {

public:

    QFontFactoryFT();
    virtual ~QFontFactoryFT();

    QRenderedFont * get(const QFontDef &,QDiskFont *);
    virtual void load(QDiskFont *) const;
    virtual void unload(QDiskFont *);
    virtual QString name();

private:

    friend class QRenderedFontFT;
    FT_Library library;
};

#endif // QT_NO_FREETYPE

#endif // QFONTFACTORYTTF_QWS_H

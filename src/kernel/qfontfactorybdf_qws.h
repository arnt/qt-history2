/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QFontFactory for Truetype class for QWS
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTFACTORY_BDF_H
#define QFONTFACTORY_BDF_H

#ifndef QT_H
#include "qfontmanager_qws.h"
#endif // QT_H

#if QT_FEATURE_BDF

class QFontFactoryBDF : public QFontFactory {

public:

    QFontFactoryBDF();
    virtual ~QFontFactoryBDF();

    QRenderedFont * get(const QFontDef &,QDiskFont *);
    virtual void load(QDiskFont *) const;
    virtual bool unicode(QRenderedFont *,int &);
    virtual QString name();

private:

    friend class QRenderedFontBDF;
};

#endif // QT_FEATURE_BDF

#endif // QFONTFACTORY_BDF_H

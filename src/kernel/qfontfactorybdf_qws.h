/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QFontFactory for Truetype class for QWS
**
** Created : 940721
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTFACTORY_BDF_H
#define QFONTFACTORY_BDF_H

#ifndef QT_H
#include "qfontmanager_qws.h"
#endif // QT_H

#ifndef QT_NO_BDF

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

#endif // QT_NO_BDF

#endif // QFONTFACTORY_BDF_H

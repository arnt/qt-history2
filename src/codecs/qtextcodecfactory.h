/****************************************************************************
**
** Definition of QTextCodecFactory class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTCODECFACTORY_H
#define QTEXTCODECFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODEC

class QTextCodec;

class Q_CORE_EXPORT QTextCodecFactory
{
public:
    static QTextCodec *createForName( const QString & );
    static QTextCodec *createForMib( int );
};

#endif // QT_NO_TEXTCODEC

#endif // QTEXTCODECFACTORY_H

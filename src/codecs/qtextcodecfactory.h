#ifndef QTEXTCODECFACTORY_H
#define QTEXTCODECFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODEC

class QTextCodec;

class Q_EXPORT QTextCodecFactory
{
public:
    static QTextCodec *createForName( const QString & );
    static QTextCodec *createForMib( int );
    static QStringList codecs();
};

#endif // QT_NO_TEXTCODEC

#endif // QTEXTCODECFACTORY_H

/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ASCIIVALIDATOR_H
#define ASCIIVALIDATOR_H


#include <qvalidator.h>


class AsciiValidator: public QValidator
{
    Q_OBJECT
public:
    AsciiValidator( QObject * parent, const char *name = 0 );
    AsciiValidator( bool funcName, QObject * parent, const char *name = 0 );
    AsciiValidator( const QString &allow, QObject * parent, const char *name = 0 );
    ~AsciiValidator();

    QValidator::State validate( QString &, int & ) const;

private:
    bool functionName;
    QString allowedChars;

};


#endif

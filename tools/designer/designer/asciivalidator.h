/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef ASCIIVALIDATOR_H
#define ASCIIVALIDATOR_H


#include <qvalidator.h>


class AsciiValidator: public QValidator
{
    Q_OBJECT
public:
    AsciiValidator( QWidget * parent, const char *name = 0 );
    AsciiValidator( bool funcName, QWidget * parent, const char *name = 0 );
    AsciiValidator( const QString &allow, QWidget * parent, const char *name = 0 );
    ~AsciiValidator();

    QValidator::State validate( QString &, int & ) const;

private:
    bool functionName;
    QString allowedChars;

};


#endif

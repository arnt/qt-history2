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

#ifndef DOMTOOL_H
#define DOMTOOL_H

#include <qvariant.h>
#include <qdom.h>

class DomTool : public Qt
{
public:
    static QVariant readProperty( const QDomElement& e, const QString& name, const QVariant& defValue );
    static bool hasProperty( const QDomElement& e, const QString& name );
    static QVariant elementToVariant( const QDomElement& e, const QVariant& defValue );
    static QVariant elementToVariant( const QDomElement& e, const QVariant& defValue, QString &comment );
    static QVariant readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue );
    static bool hasAttribute( const QDomElement& e, const QString& name );
    static QColor readColor( const QDomElement &e );

};


#endif // DOMTOOL_H

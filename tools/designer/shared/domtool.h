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

#ifndef DOMTOOL_H
#define DOMTOOL_H

#include <qvariant.h>
#include <qnamespace.h>

class QDomElement;
class QDomDocument;
class QDomNodeList;
template <class Key, class T> class QMap;

class DomTool : public Qt
{
public:
    static QVariant readProperty( const QDomElement& e, const QString& name, const QVariant& defValue );
    static QVariant readProperty( const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment );
    static bool hasProperty( const QDomElement& e, const QString& name );
    static QStringList propertiesOfType( const QDomElement& e, const QString& type );
    static QVariant elementToVariant( const QDomElement& e, const QVariant& defValue );
    static QVariant elementToVariant( const QDomElement& e, const QVariant& defValue, QString &comment );
    static QVariant readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue );
    static QVariant readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment );
    static bool hasAttribute( const QDomElement& e, const QString& name );
    static QColor readColor( const QDomElement &e );
    static void fixDocument( QDomDocument& );
    static void fixNodeList( QDomNodeList&, QMap<QString,QString>& );
};


#endif // DOMTOOL_H

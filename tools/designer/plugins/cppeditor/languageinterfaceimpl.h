 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef LANGUAGEINTERFACEIMPL_H
#define LANGUAGEINTERFACEIMPL_H

#include <languageinterface.h>

class LanguageInterfaceImpl : public LanguageInterface
{
public:
    LanguageInterfaceImpl();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    void functions( const QString &code, QValueList<Function> *funcs ) const;
    void initEventFunctions( QMap<QString, QString> & ) {}
    QString createFunctionStart( const QString &className, const QString &func, const QString &returnType );
    QStringList definitions() const;
    QStringList definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const;
    void setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface );
    QString createArguments( const QStringList &args );
    QString createEmptyFunction();
    bool supports( Support s ) const;
    void fileFilters( QMap<QString, QString> & ) const {}

private:
    ulong ref;

};

#endif

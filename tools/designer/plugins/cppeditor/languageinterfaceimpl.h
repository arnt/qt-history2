/**********************************************************************
**
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
    LanguageInterfaceImpl( QUnknownInterface *outer = 0 );

    ulong addRef();
    ulong release();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );

    void functions( const QString &code, QValueList<Function> *funcs ) const;
    void initEventFunctions( QMap<QString, QString> & ) {}
    QString createFunctionStart( const QString &className, const QString &func,
				 const QString &returnType, const QString &access );
    QStringList definitions() const;
    QStringList definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const;
    void setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface );
    QString createArguments( const QStringList &args );
    QString createEmptyFunction();
    bool supports( Support s ) const;
    QStringList fileFilterList() const;
    QStringList fileExtensionList() const;
    void preferedExtensions( QMap<QString, QString> &extensionMap ) const;
    void sourceProjectKeys( QStringList &keys ) const;
    QString projectKeyForExtension( const QString &extension ) const;
    QString cleanSignature( const QString &sig ) { return sig; } // #### implement me
    void saveFormCode( const QString &, const QString &,
		       const QValueList<Function> &,
		       const QStringList &,
		       const QStringList &,
		       const QStringList &,
		       const QStringList &,
		       const QValueList<Connection> & );
    void loadFormCode( const QString &, const QString &,
		       QValueList<Function> &,
		       QStringList &,
		       QStringList &,
		       QStringList &,
		       QStringList &,
		       QValueList<Connection> & );
    QString formCodeExtension() const { return ".h"; }
    bool canConnect( const QString &signal, const QString &slot );
    void compressProject( const QString &, const QString &, bool ) {}
    QString uncompressProject( const QString &, const QString & ) { return QString::null; }
    QString aboutText() const { return ""; }

private:
    QUnknownInterface *parent;
    ulong ref;

};

#endif

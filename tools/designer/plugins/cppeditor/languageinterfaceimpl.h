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

    void functions( const QString &code, QList<Function> *funcs ) const;
    void connections( const QString &, QList<Connection> * ) const {};
    QString createFunctionStart( const QString &className, const QString &func,
				 const QString &returnType, const QString &access );
    QStringList definitions() const;
    QStringList definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const;
    void setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface );
    QString createArguments( const QString & ) { return QString::null; }
    QString createEmptyFunction();
    bool supports( Support s ) const;
    QStringList fileFilterList() const;
    QStringList fileExtensionList() const;
    void preferedExtensions( QMap<QString, QString> &extensionMap ) const;
    void sourceProjectKeys( QStringList &keys ) const;
    QString projectKeyForExtension( const QString &extension ) const;
    QString cleanSignature( const QString &sig ) { return sig; } // #### implement me
    void loadFormCode( const QString &, const QString &,
		       QList<Function> &,
		       QStringList &,
		       QList<Connection> & );
    QString formCodeExtension() const { return ".h"; }
    bool canConnect( const QString &signal, const QString &slot );
    void compressProject( const QString &, const QString &, bool ) {}
    QString uncompressProject( const QString &, const QString & ) { return QString::null; }
    QString aboutText() const { return ""; }

    void addConnection( const QString &, const QString &,
			const QString &, const QString &,
			QString * ) {}
    void removeConnection( const QString &, const QString &,
			   const QString &, const QString &,
		
	   QString * ) {}
    QStringList signalNames( QObject *obj ) const;

private:
    QUnknownInterface *parent;
    ulong ref;

};

#endif

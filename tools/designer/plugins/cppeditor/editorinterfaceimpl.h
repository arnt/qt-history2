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

#ifndef EDITORINTERFACEIMPL_H
#define EDITORINTERFACEIMPL_H

#include <editorinterface.h>

class ViewManager;

class EditorInterfaceImpl : public EditorInterface
{
public:
    EditorInterfaceImpl();
    virtual ~EditorInterfaceImpl();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QWidget *editor( QWidget *parent, QUnknownInterface *designerIface );

    void setText( const QString &txt );
    QString text() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    bool find( const QString &expr, bool cs, bool wo, bool forward );
    void indent();
    void scrollTo( const QString &txt );
    void splitView();
    void setContext( QObjectList *toplevels, QObject *this_ );
    void readSettings();

    void setError( int line );

private:
    ViewManager *viewManager;

    unsigned long ref;
};

#endif

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

#ifndef EDITORINTERFACEIMPL_H
#define EDITORINTERFACEIMPL_H

#include <editorinterface.h>
#include <qobject.h>
#include <qguardedptr.h>

class QTimer;
class ViewManager;
struct DesignerInterface;

class EditorInterfaceImpl : public QObject, public EditorInterface
{
    Q_OBJECT

public:
    EditorInterfaceImpl();
    virtual ~EditorInterfaceImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT;

    QWidget *editor( QWidget *parent, QUnknownInterface *designerIface );

    void setText( const QString &txt );
    QString text() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    bool find( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor );
    bool replace( const QString &find, const QString &replace, bool cs, bool wo, bool forward, bool startAtCursor, bool replaceAll );
    void gotoLine( int line );
    void indent();
    void scrollTo( const QString &txt, const QString &first );
    void splitView();
    void setContext( QObjectList *toplevels, QObject *this_ );
    void readSettings();

    void setError( int line );
    void setStep( int ) {}
    void clearStep() {}
    void setModified( bool m );
    bool isModified() const;
    void setMode( Mode ) {}

    int numLines() const;
    void breakPoints( QValueList<int> & ) const {}
    void setBreakPoints( const QValueList<int> & ) {}
    void onBreakPointChange( QObject *receiver, const char *slot );
    void clearStackFrame() {}
    void setStackFrame( int ) {}

protected:
    bool eventFilter( QObject*, QEvent* );

private slots:
    void modificationChanged( bool m );
    void intervalChanged();
    void update();

private:
    QGuardedPtr<ViewManager> viewManager;
    DesignerInterface *dIface;
    QTimer *updateTimer;

};

#endif

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

    QWidget *editor( bool readonly, QWidget *parent, QUnknownInterface *designerIface );

    void setText( const QString &txt );
    QString text() const;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
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
    void setContext( QObject *this_ );
    void readSettings();

    void setError( int line );
    void setStep( int ) {}
    void clearStep() {}
    void setModified( bool m );
    bool isModified() const;
    void setMode( Mode ) {}

    int numLines() const;
    void breakPoints( QList<uint> & ) const {}
    void setBreakPoints( const QList<uint> & ) {}
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

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

#ifndef HIRARCHYVIEW_H
#define HIRARCHYVIEW_H

#include <qvariant.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qlist.h>

class HierarchyView;
class FormWindow;
class QResizeEvent;
class QCloseEvent;
class QPopupMenu;
class QTabWidget;
class QKeyEvent;
class QMouseEvent;
class QWizard;

class HierarchyItem : public QListViewItem
{
public:
    HierarchyItem( QListViewItem *parent, const QString &txt1, const QString &txt2, const QString &txt3 );
    HierarchyItem( QListView *parent, const QString &txt1, const QString &txt2, const QString &txt3 );

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    void updateBackColor();

    void setWidget( QWidget *w );
    QWidget *widget() const;

    void setText( int col, const QString &txt ) { if ( !txt.isEmpty() ) QListViewItem::setText( col, txt ); }

private:
    void okRename();
    void cancelRename();

private:
    QColor backgroundColor();
    QColor backColor;
    QWidget *wid;

};

class HierarchyList : public QListView
{
    Q_OBJECT

public:
    HierarchyList( QWidget *parent, HierarchyView *view, bool doConnects = TRUE );

    virtual void setup();
    virtual void setCurrent( QWidget *w );
    void setOpen( QListViewItem *i, bool b );
    void changeNameOf( QWidget *w, const QString &name );
    void changeDatabaseOf( QWidget *w, const QString &info );

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseReleaseEvent( QMouseEvent *e );

public slots:
    void addTabPage();
    void removeTabPage();

private:
    void insertObject( QObject *o, QListViewItem *parent );
    QWidget *findWidget( QListViewItem *i );
    QListViewItem *findItem( QWidget *w );
    QWidget *current() const;

private slots:
    virtual void objectClicked( QListViewItem *i );
    virtual void showRMBMenu( QListViewItem *, const QPoint & );

protected:
    HierarchyView *hierarchyView;
    QPopupMenu *normalMenu, *tabWidgetMenu;
    bool deselect;

};

class FunctionList : public HierarchyList
{
    Q_OBJECT

public:
    FunctionList( QWidget *parent, HierarchyView *view );

    void setup();
    void refreshFunctions( bool doDelete = TRUE );
    void setCurrent( QWidget *w );

private:
    void save( QListViewItem *p );

private slots:
    void objectClicked( QListViewItem *i );
    void showRMBMenu( QListViewItem *, const QPoint & );
    void renamed( QListViewItem *i );

};


class HierarchyView : public QTabWidget
{
    Q_OBJECT

public:
    HierarchyView( QWidget *parent );

    void setFormWindow( FormWindow *fw, QWidget *w );
    FormWindow *formWindow() const;

    void widgetInserted( QWidget *w );
    void widgetRemoved( QWidget *w );
    void widgetsInserted( const QWidgetList &l );
    void widgetsRemoved( const QWidgetList &l );
    void namePropertyChanged( QWidget *w, const QVariant &old );
    void databasePropertyChanged( QWidget *w, const QStringList& info );
    void tabsChanged( QTabWidget *w );
    void pagesChanged( QWizard *w );
    void rebuild();
    void closed( FormWindow *fw );
    void updateFunctionList();

    FunctionList *functionList() const { return fList; }

protected:
    void closeEvent( QCloseEvent *e );

signals:
    void hidden();

private:
    FormWindow *formwindow;
    HierarchyList *listview;
    FunctionList *fList;

};


#endif

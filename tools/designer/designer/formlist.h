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

#ifndef FORMLIST_H
#define FORMLIST_H

#include <qlistview.h>

class FormWindow;
class QResizeEvent;
class QCloseEvent;
class QDropEvent;
class QDragMoveEvent;
class QDragEnterEvent;
class MainWindow;
class Project;
class SourceFile;
class QCompletionEdit;
class SourceEditor;

class FormListItem : public QListViewItem
{
public:
    enum Type { Parent = 1001, Form, Image, Source };

    FormListItem( QListView *parent );
    FormListItem( QListViewItem *parent, const QString &form, const QString &file, FormWindow *fw );
    FormListItem( QListViewItem *parent, const QString &file, SourceFile *fl );

    void setFormWindow( FormWindow *fw ) { formwindow = fw; }
    FormWindow *formWindow() const { return formwindow; }

    SourceFile *sourceFile() const { return sourcefile; }

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    void updateBackColor();

    void setType( Type t ) { this->t = t; }
    Type type() const { return t; }
    int rtti() const { return (int)type(); }

private:
    FormWindow *formwindow;
    SourceFile *sourcefile;
    QColor backgroundColor();
    QColor backColor;
    Type t;

};

class FormList : public QListView
{
    Q_OBJECT

public:
    FormList( QWidget *parent , MainWindow *mw, Project *pro );

    void setProject( Project *pro );
    void addForm( FormWindow *fw );
    void closed( FormWindow *fw );

    void contentsDropEvent( QDropEvent *e );
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );

    void drawContentsOffset( QPainter *p, int ox, int oy,
			     int cx, int cy, int cw, int ch ) {
	setUpdatesEnabled( FALSE );
	triggerUpdate();
	setUpdatesEnabled( TRUE );
	QListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    }

    void removeForm( FormWindow *fw );
    void formNameChanged( FormWindow *fw );

    void setBufferEdit( QCompletionEdit *edit );

public slots:
    void modificationChanged( bool m, QObject *obj );
    void modificationChanged( bool m, FormWindow *fw ) { modificationChanged( m, (QObject*)fw ); }
    void fileNameChanged( const QString &s, FormWindow *fw );
    void activeFormChanged( FormWindow *fw );
    void activeEditorChanged( SourceEditor *se );
    void nameChanged( FormWindow *fw );

protected:
    void closeEvent( QCloseEvent *e );

signals:
    void hidden();

private slots:
    void itemClicked( int, QListViewItem *i );
    void rmbClicked( QListViewItem *i );
    void bufferChosen( const QString &buffer );

private:
    FormListItem *findItem( FormWindow *fw );
    FormListItem *findItem( SourceFile *sf );

private:
    MainWindow *mainWindow;
    Project *project;
    FormListItem *formsParent;
    FormListItem *imageParent;
    FormListItem *sourceParent;
    QCompletionEdit *bufferEdit;

};

#endif

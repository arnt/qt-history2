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

#ifndef WORKSPACE_H
#define WORKSPACE_H

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
class FormFile;
class QCompletionEdit;
class SourceEditor;

class WorkspaceItem : public QListViewItem
{
public:
    enum Type { ProjectType, FormFileType, FormSourceType, SourceFileType };

    WorkspaceItem( QListView *parent, Project* p );
    WorkspaceItem( QListViewItem *parent, SourceFile* sf );
    WorkspaceItem( QListViewItem *parent, FormFile* ff, Type t = FormFileType );

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    void updateBackColor();

    Type type() const { return t; }

    bool isModified() const;

    QString text( int ) const;

    void fillCompletionList( QStringList& completion );

    QString key( int, bool ) const; // column sorting key

    Project* project;
    SourceFile* sourceFile;
    FormFile* formFile;
    
    bool autoOpen;

private:
    void init();

    QColor backgroundColor();
    QColor backColor;
    Type t;

};

class Workspace : public QListView
{
    Q_OBJECT

public:
    Workspace( QWidget *parent , MainWindow *mw );

    void setCurrentProject( Project *pro );
//     void addForm( FormWindow *fw );
//     void closed( FormWindow *fw );

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

//     void removeFormFromProject( FormWindow *fw );
//     void removeFormFromProject( const QString &file );
//     void removeFormFromProject( QListViewItem *i );
//     void removeSourceFromProject( const QString &file );
//     void removeSourceFromProject( QListViewItem *i );
//     void formNameChanged( FormWindow *fw );

    void setBufferEdit( QCompletionEdit *edit );

//     void openForm( const QString &filename );

public slots:

    void update();
    void update( FormFile* );

    void activeFormChanged( FormWindow *fw );
    void activeEditorChanged( SourceEditor *se );

protected:
    void closeEvent( QCloseEvent *e );
    bool eventFilter( QObject *, QEvent * );


private slots:
    void itemClicked( int, QListViewItem *i );
    void rmbClicked( QListViewItem *i );
    void bufferChosen( const QString &buffer );

    void projectDestroyed( QObject* );

    void sourceFileAdded( SourceFile* );
    void sourceFileRemoved( SourceFile* );

    void formFileAdded( FormFile* );
    void formFileRemoved( FormFile* );

private:
    WorkspaceItem *findItem( FormFile *ff );
    WorkspaceItem *findItem( SourceFile *sf );
    
    void closeAutoOpenItems();

private:
    MainWindow *mainWindow;
    Project *project;
    WorkspaceItem *projectItem;
    QCompletionEdit *bufferEdit;
    bool blockNewForms;
    void updateBufferEdit();
    bool completionDirty;

};

#endif

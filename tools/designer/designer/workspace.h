/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    enum Type { ProjectType, FormFileType, FormSourceType, SourceFileType, ObjectType };

    WorkspaceItem( QListView *parent, Project* p );
    WorkspaceItem( QListViewItem *parent, SourceFile* sf );
    WorkspaceItem( QListViewItem *parent, FormFile* ff, Type t = FormFileType );
    WorkspaceItem( QListViewItem *parent, QObject *o, Project *p );

    void paintCell( QPainter *p, const QPalette &pal, int column, int width, int align );

    Type type() const { return t; }

    bool isModified() const;

    QString text( int ) const;

    void fillCompletionList( QStringList& completion );
    bool checkCompletion( const QString& completion );

    QString key( int, bool ) const; // column sorting key

    Project* project;
    SourceFile* sourceFile;
    FormFile* formFile;
    QObject *object;

    void setOpen( bool );

    void setAutoOpen( bool );
    bool isAutoOpen() const { return isOpen() && autoOpen; }

    bool useOddColor;

private:
    void init();
    bool autoOpen;
    QColor backgroundColor();
    Type t;
};

class Workspace : public QListView
{
    Q_OBJECT

public:
    Workspace( QWidget *parent , MainWindow *mw );

    void setCurrentProject( Project *pro );

    void contentsDropEvent( QDropEvent *e );
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );

    void setBufferEdit( QCompletionEdit *edit );

public slots:

    void update();
    void update( FormFile* );

    void activeFormChanged( FormWindow *fw );
    void activeEditorChanged( SourceEditor *se );

protected:
    void closeEvent( QCloseEvent *e );
    bool eventFilter( QObject *, QEvent * );


private slots:
    void itemClicked( int, QListViewItem *i, const QPoint& pos  );
    void itemDoubleClicked( QListViewItem *i );
    void rmbClicked( QListViewItem *i, const QPoint& pos  );
    void bufferChosen( const QString &buffer );

    void projectDestroyed( QObject* );

    void sourceFileAdded( SourceFile* );
    void sourceFileRemoved( SourceFile* );

    void formFileAdded( FormFile* );
    void formFileRemoved( FormFile* );

    void objectAdded( QObject* );
    void objectRemoved( QObject * );

private:
    WorkspaceItem *findItem( FormFile *ff );
    WorkspaceItem *findItem( SourceFile *sf );
    WorkspaceItem *findItem( QObject *o );

    void closeAutoOpenItems();

private:
    MainWindow *mainWindow;
    Project *project;
    WorkspaceItem *projectItem;
    QCompletionEdit *bufferEdit;
    bool blockNewForms;
    void updateBufferEdit();
    bool completionDirty;
    void updateColors();

};

#endif

#ifndef ACTIONDND_H
#define ACTIONDND_H

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qmap.h>
#include <qaction.h>

class QDesignerPopupMenu;

class QDesignerAction : public QAction
{
    Q_OBJECT

public:
    QDesignerAction( QObject *parent )
	: QAction( parent ), wid( 0 ), idx( -1 ) {}

    QWidget *widget() const { return wid; }
    int index() const { return idx; }

protected:
    void addedTo( QWidget *w, QWidget * ) {
	wid = w;
    }
    void addedTo( int index, QPopupMenu * ) {
	idx = index;
    }

private:
    QWidget *wid;
    int idx;

};

class QDesignerToolBar : public QToolBar
{
    Q_OBJECT

public:
    QDesignerToolBar( QMainWindow *mw );
    QDesignerToolBar( QMainWindow *mw, Dock dock );
    QList<QDesignerAction> insertedActions() const { return actionList; }
    void addAction( QDesignerAction *a );

protected:
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif

private slots:
    void actionRemoved();

private:
    void drawIndicator( const QPoint &pos );
    QPoint calcIndicatorPos( const QPoint &pos );
    void reInsert();

private:
    QPoint lastIndicatorPos;
    QWidget *insertAnchor;
    bool afterAnchor;
    QList<QDesignerAction> actionList;
    QMap<QWidget*, QDesignerAction*> actionMap;

};

class QDesignerMenuBar : public QMenuBar
{
    Q_OBJECT
    friend class QDesignerPopupMenu;

public:
    QDesignerMenuBar( QWidget *mw );

protected:
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif

};

class QDesignerPopupMenu : public QPopupMenu
{
    Q_OBJECT

public:
    QDesignerPopupMenu( QWidget *w );
    QList<QDesignerAction> insertedActions() const { return actionList; }
    void addAction( QDesignerAction *a );

protected:
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif

private:
    void drawIndicator( const QPoint &pos );
    QPoint calcIndicatorPos( const QPoint &pos );
    void reInsert();

private:
    QPoint lastIndicatorPos;
    int insertAt;
    QList<QDesignerAction> actionList;

};

#endif

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

class QDesignerActionGroup : public QActionGroup
{
    Q_OBJECT

public:
    QDesignerActionGroup( QObject *parent )
	: QActionGroup( parent ), wid( 0 ), idx( -1 ) {}

    QWidget *widget() const { return wid; }
    QWidget *widget( QAction *a ) const { return *widgets.find( a ); }
    int index() const { return idx; }

protected:
    void addedTo( QWidget *w, QWidget * ) {
	wid = w;
    }
    void addedTo( QWidget *w, QWidget *, QAction *a ) {
	widgets.insert( a, w );
    }
    void addedTo( int index, QPopupMenu * ) {
	idx = index;
    }

private:
    QWidget *wid;
    QMap<QAction *, QWidget *> widgets;
    int idx;

};

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

class QDesignerToolBarSeparator : public QWidget
{
    Q_OBJECT

public:
    QDesignerToolBarSeparator( Orientation, QToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    Orientation orientation() const { return orient; }
public slots:
   void setOrientation( Orientation );
protected:
    void styleChange( QStyle& );
    void paintEvent( QPaintEvent * );
private:
    Orientation orient;
};


class QSeparatorAction : public QAction
{
    Q_OBJECT

public:
    QSeparatorAction( QObject *parent );

    bool addTo( QWidget *w );
    bool removeFrom( QWidget *w );
    QWidget *widget() const;

private:
    QWidget *wid;

};

class QDesignerToolBar : public QToolBar
{
    Q_OBJECT

public:
    QDesignerToolBar( QMainWindow *mw );
    QDesignerToolBar( QMainWindow *mw, Dock dock );
    QList<QAction> insertedActions() const { return actionList; }
    void addAction( QAction *a );

protected:
    bool eventFilter( QObject *, QEvent * );
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
    void buttonMousePressEvent( QMouseEvent *e, QObject *o );
    void buttonMouseMoveEvent( QMouseEvent *e, QObject *o );

private:
    QPoint lastIndicatorPos;
    QWidget *insertAnchor;
    bool afterAnchor;
    QList<QAction> actionList;
    QMap<QWidget*, QAction*> actionMap;
    QPoint dragStartPos;

};

class QDesignerMenuBar : public QMenuBar
{
    Q_OBJECT
    friend class QDesignerPopupMenu;

    Q_PROPERTY( int itemNumber WRITE setItemNumber READ itemNumber )
    Q_PROPERTY( QString itemText WRITE setItemText READ itemText )

public:
    QDesignerMenuBar( QWidget *mw );

    void setItemNumber( int num );
    int itemNumber() const;
    void setItemText( const QString &s );
    QString itemText() const;

protected:
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif

private:
    void drawIndicator( const QPoint &pos );
    QPoint calcIndicatorPos( const QPoint &pos );

private:
    int itemNum;
    QPoint dragStartPos;
    bool mousePressed;
    QPoint lastIndicatorPos;
    int insertAt;

};

class QDesignerPopupMenu : public QPopupMenu
{
    Q_OBJECT

public:
    QDesignerPopupMenu( QWidget *w );
    QList<QAction> insertedActions() const { return actionList; }
    void addAction( QAction *a );

protected:
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
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
    int insertAt;
    QList<QAction> actionList;
    QPoint dragStartPos;
    bool mousePressed;

};

#endif

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

class QDesignerToolBar : public QToolBar
{
    Q_OBJECT

public:
    QDesignerToolBar( QMainWindow *mw );

protected:
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif
    void childEvent( QChildEvent * );
    
private:
    void drawIndicator( const QPoint &pos );
    QPoint calcIndicatorPos( const QPoint &pos );
    void reInsert();
    
private:
    QPoint lastIndicatorPos;
    QWidget *insertAnchor;
    bool afterAnchor;
    QList<QAction> actionList;
    QMap<QWidget*, QAction*> actionMap;
    QAction *insertingAction;
    
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

protected:
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif

};

#endif

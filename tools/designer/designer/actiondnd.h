#ifndef ACTIONDND_H
#define ACTIONDND_H

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpixmap.h>

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
        
private:
    void drawIndicator( const QPoint &pos );
    QPoint calcIndicatorPos( const QPoint &pos );
    
private:
    QPoint lastIndicatorPos;
    QWidget *insertAnchor;
    bool afterAnchor;
    
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

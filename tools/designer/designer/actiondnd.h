#ifndef ACTIONDND_H
#define ACTIONDND_H

#include <qtoolbar.h>

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
    
};

#endif

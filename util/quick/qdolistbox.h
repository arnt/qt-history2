#ifndef QDOLISTBOX_H
#define QDOLISTBOX_H

#include <qlistbox.h>

class QDragOffListBox : public QListBox {
    Q_OBJECT
public:
    QDragOffListBox(QWidget* parent=0, const char* name=0, WFlags f=0);

protected:
    void mousePressEvent (QMouseEvent* e);
    void mouseMoveEvent (QMouseEvent* e);

signals:
    void dragged( const char* text );
    void dragged( int );

private:
    bool loaded;
};

#endif

#ifndef QUICKED_H
#define QUICKED_H

#include <qwidget.h>
#include <qptrdict.h>
#include <qlist.h>

struct DesignerInfo {
    DesignerInfo( QWidget* w, bool t );
    QWidget* widget;
    bool top;
};

class QuickEditedWidget : public QWidget {
    Q_OBJECT
public:
    QuickEditedWidget(QWidget* parent=0, const char* name=0, WFlags f=0);
    ~QuickEditedWidget();

    void monitor(QWidget* w, bool top=TRUE);

    void clearSelection();
    void select(QWidget*);
    void startManip( QWidget* decendant, QPoint child_pos );

signals:
    void changed();

protected:
    bool eventFilter(QObject* receiver, QEvent* event);
    bool mouseEventFilter( QMouseEvent*, QWidget* );

private:
    void stopManip();
    void manip(QPoint newpos);

    enum Corner { NoCorner=0, TopLeft, TopRight, BottomRight, BottomLeft };
    static Corner nearCorner( QWidget* w, QPoint p );
    static QRect cornerRect( QWidget* w, Corner );
    static QPointArray cornerPolygon( QWidget* w, Corner );
    static QRect manipRect( QRect r, Corner c, QPoint delta );
    QWidget* selectionCorner( QPoint );

    void paintOverlay( const QRect& /*cliprect*/ );
    void repaintWithChildren( int x, int y, int w, int h );
    void repaintWithChildren( const QRect& r );
    static void repaintChildren( QWidget* parent, const QRect& r );

    QPoint mapToMe( QWidget* decendant, QPoint p );
    QRect mapTo( QWidget* ancestor, QWidget* decendant, QRect r );
    QRect mapToMe( QWidget* decendant, QRect r );
    static QPoint mapTo( QWidget* ancestor, QWidget* decendant, QPoint p );
    QWidget* top(QWidget* decendant);

private: // members
    QPoint drag_last_pos;
    Corner manip_pos;
    QList<QWidget> selection;
    QWidget *primary_selection;
    QPtrDict<DesignerInfo> infoDict;
};


#endif

#ifndef QDOCKAREA_H
#define QDOCKAREA_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#include "qdockwidget.h"
#include "qlayout.h"
#include "qvaluelist.h"
#include "qlist.h"
#include "qguardedptr.h"
#endif // QT_H

class QSplitter;
class QBoxLayout;
class QDockAreaLayout;
class QMouseEvent;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QValueList<QRect>;
template class Q_EXPORT QList<QDockWidget>;
// MOC_SKIP_END
#endif

class Q_EXPORT QDockAreaLayout : public QLayout
{
    Q_OBJECT
    friend class QDockArea;

public:
    QDockAreaLayout( QWidget* parent, Qt::Orientation o, QList<QDockWidget> *wl, int space = -1, int margin = -1, const char *name = 0 )
	: QLayout( parent, space, margin, name ), orient( o ), dockWidgets( wl ), parentWidget( parent ) { init(); }
    ~QDockAreaLayout() {}

    void addItem( QLayoutItem * ) {}
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    int widthForHeight( int ) const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::NoDirection; }
    void invalidate();
    Qt::Orientation orientation() const { return orient; }
    QValueList<QRect> lineList() const { return lines; }
    QList<QDockWidget> lineStarts() const { return ls; }

protected:
    void setGeometry( const QRect& );

private:
    void init();
    int layoutItems( const QRect&, bool testonly = FALSE );
    Qt::Orientation orient;
    int cached_width, cached_height;
    int cached_hfw, cached_wfh;
    QList<QDockWidget> *dockWidgets;
    QWidget *parentWidget;
    QValueList<QRect> lines;
    QList<QDockWidget> ls;

};

class Q_EXPORT QDockArea : public QWidget
{
    Q_OBJECT

    friend class QDockWidget;
    
public:
    enum Gravity { Normal, Reverse };

    QDockArea( Orientation o, Gravity g = Normal, QWidget *parent = 0, const char *name = 0 );
    ~QDockArea();

    void moveDockWidget( QDockWidget *w, const QPoint &globalPos, const QRect &rect, bool swap );
    void removeDockWidget( QDockWidget *w, bool makeFloating, bool swap );
    void addDockWidget( QDockWidget *w, int index = -1 );
    bool hasDockWidget( QDockWidget *w, int *index = 0 );

    void invalidNextOffset( QDockWidget *dw );

    Orientation orientation() const { return orient; }
    Gravity gravity() const { return grav; }

    bool eventFilter( QObject *, QEvent * );
    bool isEmpty() const;
    void updateLayout();
    QList<QDockWidget> dockWidgetList() const;
    void lineUp( bool keepNewLines );

signals:
    void rightButtonPressed( const QPoint &globalPos );

protected:
    void mousePressEvent( QMouseEvent *e );

private:
    struct DockWidgetData
    {
	int index;
	int offset;
	int line;
	QGuardedPtr<QDockArea> area;
    };

    int findDockWidget( QDockWidget *w );
    int lineOf( int index );
    DockWidgetData *dockWidgetData( QDockWidget *w );
    void dockWidget( QDockWidget *dockWidget, DockWidgetData *data );
    
private:
    Orientation orient;
    QList<QDockWidget> *dockWidgets;
    QDockAreaLayout *layout;
    Gravity grav;
    
    
};

#endif


#ifndef QDOCKWIDGET_H
#define QDOCKWIDGET_H

#include <qframe.h>

class QDockWidgetHandle;
class QDockWidgetTitleBar;
class QPainter;
class QDockArea;

class QDockWidget : public QFrame
{
    friend class QDockWidgetHandle;
    friend class QDockWidgetTitleBar;
    Q_OBJECT

public:
    enum Place { InDock, OutsideDock };

    QDockWidget( QWidget *parent = 0, const char *name = 0 );

    virtual void setWidget( QWidget *w );
    QWidget *widget() const;

    Place place() const { return curPlace; }

    virtual void setResizeEnabled( bool b );
    bool isResizeEnabled() const;

    virtual void setCloseEnabled( bool b );
    bool isCloseEnabled() const;

    QDockArea *area() const;

protected:
    void resizeEvent( QResizeEvent *e );

private:
    void handleMoveInDock( const QPoint &pos );
    void handleMoveOutsideDock( const QPoint &pos, const QPoint &gp );
    void updateGui();

    void startRectDraw( const QPoint &so );
    void endRectDraw();
    void updatePosition( const QPoint &globalPos  );

private:
    QDockWidgetHandle *handle;
    QDockWidgetTitleBar *titleBar;
    Place curPlace;
    QWidget *wid;
    QPainter *unclippedPainter;
    QRect currRect;
    Place state;
    QDockArea *dockArea, *tmpDockArea;
    bool closeEnabled, resizeEnabled;
    Orientation startOrientation;
    QRect startRect;
    QPoint startOffset;
    
};

inline QDockArea *QDockWidget::area() const
{
    return dockArea;
}

#endif

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

    QDockArea *area() const;

    virtual void setCloseEnabled( bool b );
    bool isCloseEnabled() const;

    virtual void setResizeEnabled( bool b );
    bool isResizeEnabled() const;

    void setHorizontalStretchable( bool b );
    void setVerticalStretchable( bool b );
    bool isHorizontalStretchable() const;
    bool isVerticalStretchable() const;
    bool isStretchable() const;

    void setOffset( int o );
    int offset() const;

    void setFixedExtendWidth( int w );
    void setFixedExtendHeight( int h );
    QSize fixedExtend() const;
    bool hasFixedExtend() const;

    void setNewLine( bool b );
    bool newLine() const;
    
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    Qt::Orientation orientation() const;

signals:
    void orientationChanged( Orientation o );

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
    int addY, addX;
    bool stretchable[ 2 ];
    int offs;
    QSize fExtend;
    bool nl;
    
};

inline QDockArea *QDockWidget::area() const
{
    return dockArea;
}

#endif

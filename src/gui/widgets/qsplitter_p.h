#ifndef __QSPLITTER_P_H__
#define __QSPLITTER_P_H__

#include <private/qframe_p.h>

class QSplitterPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QSplitter);
public:
    QSplitterPrivate() : rubber(0), opaque( false ), firstShow( true ), 
			 childrenCollapsible( true ), handleWidth( 0 ) { }

    QPointer<QRubberBand> rubber;
    mutable QList<QSplitterLayoutStruct *> list;
    Orientation orient;
    bool opaque : 8;
    bool firstShow : 8;
    bool childrenCollapsible : 8;
    int handleWidth;

    inline QCOORD pick( const QPoint &p ) const
    { return orient == Qt::Horizontal ? p.x() : p.y(); }
    inline QCOORD pick( const QSize &s ) const
    { return orient == Qt::Horizontal ? s.width() : s.height(); }

    inline QCOORD trans( const QPoint &p ) const
    { return orient == Qt::Vertical ? p.x() : p.y(); }
    inline QCOORD trans( const QSize &s ) const
    { return orient == Qt::Vertical ? s.width() : s.height(); }
};


#endif /* __QSPLITTER_P_H__ */

#ifndef QSPLITTER_P_H
#define QSPLITTER_P_H

#include <private/qframe_p.h>
class QSplitterLayoutStruct;
class QSplitterPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QSplitter)
public:
    QSplitterPrivate() : rubberBand(0), opaque(false), firstShow(true),
                         childrenCollapsible(true), compatMode(false), handleWidth(0) {}

    QPointer<QRubberBand> rubberBand;
    mutable QList<QSplitterLayoutStruct *> list;
    Qt::Orientation orient;
    bool opaque : 8;
    bool firstShow : 8;
    bool childrenCollapsible : 8;
    bool compatMode : 8;
    int handleWidth;

    inline QCOORD pick(const QPoint &p) const
    { return orient == Qt::Horizontal ? p.x() : p.y(); }
    inline QCOORD pick(const QSize &s) const
    { return orient == Qt::Horizontal ? s.width() : s.height(); }

    inline QCOORD trans(const QPoint &p) const
    { return orient == Qt::Vertical ? p.x() : p.y(); }
    inline QCOORD trans(const QSize &s) const
    { return orient == Qt::Vertical ? s.width() : s.height(); }

    void init();
    void recalc(bool update = false);
    void doResize();
    void storeSizes();
    void getRange(int id, int *, int *, int *, int *);
    void addContribution(int, int *, int *, bool);
    int adjustPos(int, int, int *, int *, int *, int *);
    bool collapsible(QSplitterLayoutStruct *);
    void processChildEvents();
    QSplitterLayoutStruct *findWidget(QWidget *);
    QSplitterLayoutStruct *addWidget(QWidget *, bool prepend = false);
    void recalcId();
    void doMove(bool backwards, int pos, int id, int delta, bool upLeft,
                 bool mayCollapse);
    void setGeo(QSplitterLayoutStruct *s, int pos, int size, bool splitterMoved);
    int findWidgetJustBeforeOrJustAfter(int id, int delta, int &collapsibleSize);
    void updateHandles();

};

#endif // QSPLITTER_P_H

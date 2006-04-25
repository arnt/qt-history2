#ifndef QTGRADIENTSTOPSWIDGET_H
#define QTGRADIENTSTOPSWIDGET_H

#include <QAbstractScrollArea>

class QtGradientStopsModel;

class QtGradientStopsWidget : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
public:
    QtGradientStopsWidget(QWidget *parent = 0);
    ~QtGradientStopsWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setGradientStopsModel(QtGradientStopsModel *model);

    void setZoom(double zoom);
    double zoom() const;

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);

private:
    class QtGradientStopsWidgetPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsWidget)
    Q_DISABLE_COPY(QtGradientStopsWidget)
    Q_PRIVATE_SLOT(d_func(), void slotStopAdded(QtGradientStop *stop));
    Q_PRIVATE_SLOT(d_func(), void slotStopRemoved(QtGradientStop *stop));
    Q_PRIVATE_SLOT(d_func(), void slotStopMoved(QtGradientStop *stop, qreal newPos));
    Q_PRIVATE_SLOT(d_func(), void slotStopChanged(QtGradientStop *stop, const QColor &newColor));
    Q_PRIVATE_SLOT(d_func(), void slotStopSelected(QtGradientStop *stop, bool selected));
    Q_PRIVATE_SLOT(d_func(), void slotCurrentStopChanged(QtGradientStop *stop));
    Q_PRIVATE_SLOT(d_func(), void slotNewStop());
    Q_PRIVATE_SLOT(d_func(), void slotDelete());
    Q_PRIVATE_SLOT(d_func(), void slotSelectAll());
};

#endif

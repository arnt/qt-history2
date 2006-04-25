#ifndef QTBRUSHWIDGET_H
#define QTBRUSHWIDGET_H

#include <QWidget>

class QtBrushWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
public:
    QtBrushWidget(QWidget *parent = 0);
    ~QtBrushWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    int heightForWidth(int w) const;

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBackgroundSize(int size);

protected:
    void paintEvent(QPaintEvent *e);

private:
    class QtBrushWidgetPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushWidget)
    Q_DISABLE_COPY(QtBrushWidget)
};

#endif

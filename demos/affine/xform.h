#ifndef XFORM_H
#define XFORM_H

#include "arthurwidgets.h"
#include <qbasictimer.h>

class HoverPoints;

class XFormView : public ArthurFrame
{
    Q_OBJECT

public:
    XFormView(QWidget *parent);
    void paint(QPainter *);
    void drawVectorType(QPainter *painter);
    void drawPixmapType(QPainter *painter);
    void drawTextType(QPainter *painter);
    QSize sizeHint() const { return QSize(500, 500); }

    void mousePressEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    HoverPoints *hoverPoints() { return pts; }

    QLineEdit *textEditor;

public slots:
    void setAnimation(bool animate);
    void updateCtrlPoints(const QPolygonF &);
    void changeRotation(int rotation);
    void changeScale(int scale);
    void changeShear(int shear);

    void setVectorType();
    void setPixmapType();
    void setTextType();
    void reset();

signals:
    void rotationChanged(int rotation);
    void scaleChanged(int scale);
    void shearChanged(int shear);

protected:
    void timerEvent(QTimerEvent *e);
    void wheelEvent(QWheelEvent *);

private:
    enum XFormType { VectorType, PixmapType, TextType };

    QPolygonF ctrlPoints;
    HoverPoints *pts;
    double rotation;
    double scale;
    double shear;
    XFormType type;
    QPixmap pixmap;
    QBasicTimer timer;
};

class XFormWidget : public QWidget
{
    Q_OBJECT
public:
    XFormWidget(QWidget *parent);

private:
    XFormView *view;
};

#endif // XFORM_H

#ifndef QTGRADIENTSTOPSEDITOR_H
#define QTGRADIENTSTOPSEDITOR_H

#include <QWidget>

class QtGradientStopsEditor : public QWidget
{
    Q_OBJECT
public:
    QtGradientStopsEditor(QWidget *parent = 0);
    ~QtGradientStopsEditor();

    void setGradientStops(const QGradientStops &stops);
    QGradientStops gradientStops() const;

signals:

    void gradientStopsChanged(const QGradientStops &stops);

private:
    class QtGradientStopsEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsEditor)
    Q_DISABLE_COPY(QtGradientStopsEditor)
    Q_PRIVATE_SLOT(d_func(), void slotHsvClicked());
    Q_PRIVATE_SLOT(d_func(), void slotRgbClicked());
    Q_PRIVATE_SLOT(d_func(), void slotCurrentStopChanged(QtGradientStop *stop));
    Q_PRIVATE_SLOT(d_func(), void slotStopMoved(QtGradientStop *stop, qreal newPos));
    Q_PRIVATE_SLOT(d_func(), void slotStopChanged(QtGradientStop *stop, const QColor &newColor));
    Q_PRIVATE_SLOT(d_func(), void slotStopSelected(QtGradientStop *stop, bool selected));
    Q_PRIVATE_SLOT(d_func(), void slotStopAdded(QtGradientStop *stop));
    Q_PRIVATE_SLOT(d_func(), void slotStopRemoved(QtGradientStop *stop));
    Q_PRIVATE_SLOT(d_func(), void slotUpdatePositionSpinBox());
    Q_PRIVATE_SLOT(d_func(), void slotChangeColor(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(const QColor &color));
    //Q_PRIVATE_SLOT(d_func(), void slotChangePosition(double newPos));
    Q_PRIVATE_SLOT(d_func(), void slotChangePosition());
    Q_PRIVATE_SLOT(d_func(), void slotChangeZoom());
    Q_PRIVATE_SLOT(d_func(), void slotZoomIn());
    Q_PRIVATE_SLOT(d_func(), void slotZoomOut());
    Q_PRIVATE_SLOT(d_func(), void slotZoomAll());
};

#endif

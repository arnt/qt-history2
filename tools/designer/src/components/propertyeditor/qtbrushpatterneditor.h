#ifndef QTBRUSHPATTERNEDITOR_H
#define QTBRUSHPATTERNEDITOR_H

#include <QWidget>

class QtBrushPatternEditor : public QWidget
{
    Q_OBJECT
public:
    QtBrushPatternEditor(QWidget *parent = 0);
    ~QtBrushPatternEditor();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

private:
    class QtBrushPatternEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushPatternEditor)
    Q_DISABLE_COPY(QtBrushPatternEditor)
    Q_PRIVATE_SLOT(d_func(), void slotHsvClicked());
    Q_PRIVATE_SLOT(d_func(), void slotRgbClicked());
    Q_PRIVATE_SLOT(d_func(), void slotPatternChanged(int pattern));
    Q_PRIVATE_SLOT(d_func(), void slotChangeColor(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(const QColor &color));
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(const QColor &color));
};

#endif

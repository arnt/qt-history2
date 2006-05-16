#ifndef QTBRUSHBUTTON_H
#define QTBRUSHBUTTON_H

#include <QToolButton>

class QDesignerBrushManagerInterface;

class QtBrushButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
public:
    QtBrushButton(QWidget *parent = 0);
    ~QtBrushButton();

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QDesignerBrushManagerInterface *manager);

    void setTexture(const QPixmap &texture);
signals:
    void brushChanged(const QBrush &brush);
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
protected:
    void paintEvent(QPaintEvent *e);

private:
    class QtBrushButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushButton)
    Q_DISABLE_COPY(QtBrushButton)
    Q_PRIVATE_SLOT(d_func(), void slotEditBrush());
};

#endif

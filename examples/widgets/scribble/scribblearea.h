#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>

class ScribbleArea : public QWidget
{
    Q_OBJECT

public:
    ScribbleArea(QWidget *parent = 0);

    bool openImage(const QString &fileName);
    bool saveImage(const QString &fileName, const char *fileFormat);

    bool isModified() const { return modified; }

    QColor getPenColor();
    int getPenWidth();

    void setPenColor(const QColor &newColor);
    void setPenWidth(int newWidth);

public slots:
    void clearImage();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void drawLineTo(const QPoint &endPoint);

    bool modified;
    bool scribbling;
    int penWidth;
    QColor penColor;
    QImage image;
    QPoint startPoint;
};

#endif

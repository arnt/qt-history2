#ifndef QGLVFBVIEW_H
#define QGLVFBVIEW_H

#include <QGLWidget>
#include "qvfbviewiface.h"

class QImage;
class QTimer;
class QAnimationWriter;
struct QVFbHeader;

class QGLVFbView : public QGLWidget,
                   public QVFbViewIface
{
    Q_OBJECT
public:
    QGLVFbView(int display_id, int w, int h, int d, Rotation r, QWidget *parent = 0);
    ~QGLVFbView();

    bool animating() const { return !!animation; }
    QImage image() const;

    void setGamma(double gr, double gg, double gb);

    void skinKeyPressEvent(int code, const QString& text, bool autorep=false);
    void skinKeyReleaseEvent(int code, const QString& text, bool autorep=false);
    void skinMouseEvent(QMouseEvent *e);

    QSize sizeHint() const;

public slots:
    bool setRate(int);
    bool setZoom(double, double);
    void startAnimation(const QString &);
    void stopAnimation();

protected slots:
    void flushChanges();

protected:
    QImage getBuffer(const QRect &r, int &leading) const;
    void drawScreen();
    //virtual bool eventFilter(QObject *obj, QEvent *e);
    virtual void paintEvent(QPaintEvent *pe);
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);

private:
    void setDirty(const QRect &);
    int shmId;
    unsigned char *data;
    int viewdepth; // "faked" depth
    int rsh;
    int gsh;
    int bsh;
    int rmax;
    int gmax;
    int bmax;
    int contentsWidth;
    int contentsHeight;
    int lockId;
    QTimer *t_flush;

    QAnimationWriter *animation;
};

#endif

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVFBVIEW_H
#define QVFBVIEW_H

#include <QWidget>
#include "qvfbviewiface.h"

class QImage;
class QTimer;
class QAnimationWriter;
struct QVFbHeader;

class QVFbView : public QWidget,
                 public QVFbViewIface
{
    Q_OBJECT
public:
    QVFbView( int display_id, int w, int h, int d, Rotation r, QWidget *parent = 0,
		Qt::WFlags wflags = 0 );
    ~QVFbView();

    bool animating() const { return !!animation; }
    QImage image() const;

    void setGamma(double gr, double gg, double gb);

    void skinKeyPressEvent( int code, const QString& text, bool autorep=FALSE );
    void skinKeyReleaseEvent( int code, const QString& text, bool autorep=FALSE );
    void skinMouseEvent( QMouseEvent *e );

    QSize sizeHint() const;

public slots:
    bool setRate( int );
    bool setZoom( double, double );
    void startAnimation(const QString &);
    void stopAnimation();

protected slots:
    void flushChanges();

protected:
    QImage getBuffer( const QRect &r, int &leading ) const;
    void drawScreen();
    //virtual bool eventFilter( QObject *obj, QEvent *e );
    virtual void paintEvent( QPaintEvent *pe );
    virtual void contextMenuEvent( QContextMenuEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void wheelEvent( QWheelEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

private:
    void setDirty( const QRect& );
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

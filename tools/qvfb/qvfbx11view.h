/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVFBX11VIEW_H
#define QVFBX11VIEW_H

#include "qvfbview.h"

class X11KeyFaker;
class QProcess;
class QTemporaryFile;

class QVFbX11View : public QVFbAbstractView
{
    Q_OBJECT
public:
    QVFbX11View( int id, int w, int h, int d, Rotation r, QWidget *parent = 0);
    virtual ~QVFbX11View();

    QString xServerPath() const { return xserver; }
    void setXServerPath(const QString& path) { xserver = path; }

    int displayId() const;
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;
    Rotation displayRotation() const;

    void skinKeyPressEvent( int code, const QString& text, bool autorep=FALSE );
    void skinKeyReleaseEvent( int code, const QString& text, bool autorep=FALSE );

    void setGamma(double gr, double gg, double gb);
    double gammaRed() const;
    double gammaGreen() const;
    double gammaBlue() const;
    void getGamma(int i, QRgb& rgb);

    bool touchScreenEmulation() const;
    bool lcdScreenEmulation() const;
    int rate();
    bool animating() const;
    QImage image() const;
    void setRate(int);

    double zoomH() const;
    double zoomV() const;

    QSize sizeHint() const;

public slots:
    void setTouchscreenEmulation( bool );
    void setLcdScreenEmulation( bool );
    void setZoom( double, double );
    void startAnimation( const QString& );
    void stopAnimation();

protected:
    void showEvent(QShowEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

private slots:
    void startXnest();
    void xnestStopped();
    void startKeyFaker();

private:
    int id, w, h, d;
    Rotation rotation;
    double gr, gg, gb;
    bool touchscreen, lcd;
    X11KeyFaker *keyFaker;
    QProcess *xnest;
    QTemporaryFile *serverAuthFile;
    bool shutdown;
    QString xserver;
};

#endif

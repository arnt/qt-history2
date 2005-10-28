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

#ifndef QVFBVIEWCOMPOSITE_H
#define QVFBVIEWCOMPOSITE_H

#include <QColor>
#include <QString>
#include <QImage>

/* An assumption that something will inherit both this and QWidget */
class QVFbDisplay
{
public:
    enum Rotation { Rot0, Rot90, Rot180, Rot270 };
public:
    QVFbDisplay(Rotation r, QVFbViewProtocol *);
    virtual ~QVFbDisplay();


    Rotation displayRotation() const;

    bool touchScreenEmulation() const;
    bool lcdScreenEmulation() const;

    void   setGamma(double gr, double gg, double gb);
    double gammaRed() const;
    double gammaGreen() const;
    double gammaBlue() const;

    void getGamma(int i, QRgb& rgb);

    double zoomH() const;
    double zoomV() const;

    /* screen shot? */
    virtual QImage image() const =0;

    virtual void skinKeyPressEvent( int code, const QString& text,
                                    bool autorep=FALSE ) =0;
    virtual void skinKeyReleaseEvent( int code, const QString& text,
                                      bool autorep=FALSE ) =0;
    virtual void skinMouseEvent( QMouseEvent *e ) =0;
public:
    virtual bool setTouchscreenEmulation(bool);
    virtual bool setLcdScreenEmulation(bool);
    virtual bool setZoom(double, double);

public:
    static QRect mapToDevice(const QRect &r, const QSize &s, Rotation rotation);

protected:
    // helper functions for actually doing the display.
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;

    void sendMouseData(const QPoint &pos, int buttons, int wheel);
    void sendKeyboardData(int unicode, int keycode, int modifiers,
                          bool press, bool repeat);
protected:
    QVFbViewProtocol *m_view;
    Rotation m_rotation;
    bool m_emulateTouchscreen;
    bool m_emulateLcdScreen;
    QRgb *m_gammatable;
    double m_gred, m_ggreen, m_gblue;
    double m_hzm, m_vzm;
    int m_rmax;
};

inline int QVFbDisplay::displayId() const
{
    if (!m_view)
        return 0;
    return m_view->id();
}

inline int QVFbDisplay::displayWidth() const
{
    if (!m_view)
        return 0;
    if ( (int)m_rotation & 0x01 ) 
        return m_view->size().height();
    else
        return m_view->size().width();
}

inline int QVFbDisplay::displayHeight() const
{
    if (!m_view)
        return 0;
    if ( (int)m_rotation & 0x01 ) 
        return m_view->size().width();
    else
        return m_view->size().height();
}

inline int QVFbDisplay::displayDepth() const
{
    if (!m_view)
        return 0;
    return m_view->depth();
}

inline QVFbDisplay::Rotation QVFbDisplay::displayRotation() const
{
    return m_rotation;
}

inline bool QVFbDisplay::touchScreenEmulation() const
{
    return m_emulateTouchscreen;
}

inline bool QVFbDisplay::lcdScreenEmulation() const
{
    return m_emulateLcdScreen;
}

inline double QVFbDisplay::gammaRed() const
{
    return m_gred;
}

inline double QVFbDisplay::gammaGreen() const
{
    return m_ggreen;
}

inline double QVFbDisplay::gammaBlue() const
{
    return m_gblue;
}

inline double QVFbDisplay::zoomH() const
{
    return m_hzm;
}

inline double QVFbDisplay::zoomV() const
{
    return m_vzm;
}

#endif

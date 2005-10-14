#ifndef QVFBVIEWIFACE_H
#define QVFBVIEWIFACE_H

#include <QColor>
#include <QString>
#include <QImage>

class QVFbHeader;
class QMouseEvent;
class QPoint;

class QVFbViewIface
{
public:
    enum Rotation { Rot0, Rot90, Rot180, Rot270 };
public:
    QVFbViewIface(int display_id, int w, int h, int d, Rotation r);
    virtual ~QVFbViewIface();

    int displayId() const;
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;
    Rotation displayRotation() const;

    bool touchScreenEmulation() const;
    bool lcdScreenEmulation() const;
    int rate() const;

    void   setGamma(double gr, double gg, double gb);
    double gammaRed() const;
    double gammaGreen() const;
    double gammaBlue() const;

    void getGamma(int i, QRgb& rgb);

    double zoomH() const;
    double zoomV() const;

    virtual QWidget *widget() = 0;
    virtual QImage image() const =0;

    virtual void skinKeyPressEvent( int code, const QString& text,
                                    bool autorep=FALSE ) =0;
    virtual void skinKeyReleaseEvent( int code, const QString& text,
                                      bool autorep=FALSE ) =0;
    virtual void skinMouseEvent( QMouseEvent *e ) =0;
public:
    virtual bool setTouchscreenEmulation(bool);
    virtual bool setLcdScreenEmulation(bool);
    virtual bool setRate(int);
    virtual bool setZoom(double, double);

public:
    static QRect mapToDevice(const QRect &r, const QSize &s, Rotation rotation);

protected:
    void sendMouseData(const QPoint &pos, int buttons, int wheel);
    void sendKeyboardData(int unicode, int keycode, int modifiers,
                          bool press, bool repeat);
protected:
    int m_displayId;
    int m_width, m_height;
    int m_viewDepth;
    Rotation m_rotation;
    bool m_emulateTouchscreen;
    bool m_emulateLcdScreen;
    int m_refreshRate;
    QRgb *m_gammatable;
    double m_gred, m_ggreen, m_gblue;
    double m_hzm, m_vzm;

    int m_mouseFd;
    int m_keyboardFd;
    QString m_mousePipe;
    QString m_keyboardPipe;

    int m_rmax;

    QVFbHeader *m_hdr;
};

inline int QVFbViewIface::displayId() const
{
    return m_displayId;
}

inline int QVFbViewIface::displayWidth() const
{
    return ( (int)m_rotation & 0x01 ) ? m_height : m_width;
}

inline int QVFbViewIface::displayHeight() const
{
    return ( (int)m_rotation & 0x01 ) ? m_width : m_height;
}

inline int QVFbViewIface::displayDepth() const
{
    return m_viewDepth;
}

inline QVFbViewIface::Rotation QVFbViewIface::displayRotation() const
{
    return m_rotation;
}

inline bool QVFbViewIface::touchScreenEmulation() const
{
    return m_emulateTouchscreen;
}

inline bool QVFbViewIface::lcdScreenEmulation() const
{
    return m_emulateLcdScreen;
}

inline int QVFbViewIface::rate() const
{
    return m_refreshRate;
}

inline double QVFbViewIface::gammaRed() const
{
    return m_gred;
}

inline double QVFbViewIface::gammaGreen() const
{
    return m_ggreen;
}

inline double QVFbViewIface::gammaBlue() const
{
    return m_gblue;
}

inline double QVFbViewIface::zoomH() const
{
    return m_hzm;
}

inline double QVFbViewIface::zoomV() const
{
    return m_vzm;
}

#endif

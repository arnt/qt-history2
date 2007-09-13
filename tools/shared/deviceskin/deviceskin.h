/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SKIN_H
#define SKIN_H

#include <QtGui/QWidget>
#include <QtGui/QPolygon>
#include <QtGui/QRegion>
#include <QtGui/QPixmap>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

namespace qvfb_internal {
    class CursorWindow;
}

class QTextStream;

// ------- Button Area
struct DeviceSkinButtonArea {
    DeviceSkinButtonArea();
    QString name;
    int	keyCode;
    QPolygon area;
    QString text;
    bool activeWhenClosed;
};

// -------- Parameters
struct DeviceSkinParameters {
    enum ReadMode { ReadAll, ReadSizeOnly };
    bool read(const QString &skinDirectory,  ReadMode rm,  QString *errorMessage);
    bool read(QTextStream &ts, ReadMode rm, QString *errorMessage);

    QSize screenSize() const { return screenRect.size(); }
    QSize secondaryScreenSize() const;
    bool hasSecondaryScreen() const;

    QString skinImageUpFileName;
    QString skinImageDownFileName;
    QString skinImageClosedFileName;
    QString skinCursorFileName;

    QImage skinImageUp;
    QImage skinImageDown;
    QImage skinImageClosed;
    QImage skinCursor;

    QRect screenRect;
    QRect backScreenRect;
    QRect closedScreenRect;
    QPoint cursorHot;
    QVector<DeviceSkinButtonArea> buttonAreas;

    int joystick;
    QString prefix;
};

// --------- Skin Widget
class DeviceSkin : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceSkin(const DeviceSkinParameters &parameters,  QWidget *p );
    ~DeviceSkin( );

    QWidget *view() const { return m_view; }
    void setView( QWidget *v );

    QWidget *secondaryView() const { return m_secondaryView; }
    void setSecondaryView( QWidget *v );

    void setZoom( double );

    bool hasCursor() const;

    QString prefix() const  {return m_parameters.prefix;}

signals:
    void popupMenu();
    void skinKeyPressEvent(int code, const QString& text, bool autorep);
    void skinKeyReleaseEvent(int code, const QString& text, bool autorep);

protected slots:
    void skinKeyRepeat();
    void moveParent();

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent * );

private:
    void calcRegions();
    void flip(bool open);
    void updateSecondaryScreen();
    void loadImages();
    void startPress(int);
    void endPress();

    const DeviceSkinParameters m_parameters;
    QVector<QRegion> buttonRegions;
    QPixmap skinImageUp;
    QPixmap skinImageDown;
    QPixmap skinImageClosed;
    QPixmap skinCursor;
    QWidget *parent;
    QWidget  *m_view;
    QWidget *m_secondaryView;
    QPoint parentpos;
    QPoint clickPos;
    bool buttonPressed;
    int buttonIndex;
    double zoom;
    qvfb_internal::CursorWindow *cursorw;

    bool joydown;
    QTimer *t_skinkey;
    QTimer *t_parentmove;
    int onjoyrelease;

    bool flipped_open;
};

QT_END_NAMESPACE

#endif

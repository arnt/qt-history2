#ifndef _TRAYAPP_H
#define _TRAYAPP_H

#include <qobject.h>
#include <qpixmap.h>

class QPopupMenu;

class QTrayIcon : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
    Q_PROPERTY( QPixmap icon READ icon WRITE setIcon )

public:
    QTrayIcon( QObject *parent, const char *name = 0 );
    ~QTrayIcon();

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;

    QPixmap		icon() const;
    QString		toolTip() const;

public slots:
    void		setIcon( const QPixmap &icon );
    void		setToolTip( const QString &tip );

signals:
    void		clicked( const QPoint& );
    void		doubleClicked( const QPoint& );

private slots:
    void		doPopup( const QPoint& );

private:
    QPopupMenu* pop;
    QPixmap pm;
    QString tip;

    // system-dependant part
    class QTrayIconPrivate;
    QTrayIconPrivate *d;
    void sysInstall();
    void sysRemove();
    void sysUpdateIcon();
    void sysUpdateToolTip();
};

#endif

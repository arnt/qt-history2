#ifndef _TRAYAPP_H
#define _TRAYAPP_H

#include <qapplication.h>

class QPopupMenu;
class QPixmap;

class QTrayApplication : public QApplication
{
    Q_OBJECT
    Q_PROPERTY( QPixmap trayIcon READ trayIcon WRITE setTrayIcon )
    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )

public:
    QTrayApplication( int argc, char **argv );
    ~QTrayApplication();

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;

    QPixmap		trayIcon() const;
    QString		toolTip() const;

signals:
    void		clicked( const QPoint& );
    void		doubleClicked( const QPoint& );

public slots:
    void		setTrayIcon( const QPixmap &icon );
    void		setToolTip( const QString &tip );

			// Provided for removal of the icon
    void		quit();

protected:
    bool		winEventFilter( MSG * );

private:
    class QTrayApplicationPrivate;

    QTrayApplicationPrivate *d;
};

#endif

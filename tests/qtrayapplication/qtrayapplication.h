#ifndef _TRAYAPP_H
#define _TRAYAPP_H

#include <qapplication.h>

class QPopupMenu;
class QPixmap;

class QTrayApplication : public QApplication
{
    Q_OBJECT
    Q_PROPERTY( QPixmap icon READ icon WRITE setIcon )
    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )

public:
    QTrayApplication( int argc, char **argv );

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;

    QPixmap		icon() const;
    QString		toolTip() const;

signals:
    void		clicked( const QPoint& );
    void		doubleClicked( const QPoint& );

public slots:
    void		setIcon( const QPixmap &icon );
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

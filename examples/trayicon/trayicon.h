#ifndef TRAYICON_H
#define TRAYICON_H

#ifndef QT_H
#include <qobject.h>
#include <qimage.h>
#endif // QT_H

class QPopupMenu;

class TrayIcon : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
    Q_PROPERTY( QImage icon READ icon WRITE setIcon )

public:
    TrayIcon( QObject *parent = 0, const char *name = 0 );
    TrayIcon( const QImage &, const QString &, QPopupMenu *popup = 0, QObject *parent = 0, const char *name = 0 );
    ~TrayIcon();

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;

    QImage		icon() const;
    QString		toolTip() const;

public slots:
    void		setIcon( const QImage &icon );
    void		setToolTip( const QString &tip );

    void		show();
    void		hide();

signals:
    void		clicked( const QPoint& );
    void		doubleClicked( const QPoint& );

protected:
    bool		event( QEvent * );
    virtual void	mouseMoveEvent( QMouseEvent *e );
    virtual void	mousePressEvent( QMouseEvent *e );
    virtual void	mouseReleaseEvent( QMouseEvent *e );
    virtual void	mouseDoubleClickEvent( QMouseEvent *e );

private:
    QPopupMenu *pop;
    QImage img;
    QString tip;

    // system-dependant part
    class TrayIconPrivate;
    TrayIconPrivate *d;
    void sysInstall();
    void sysRemove();
    void sysUpdateIcon();
    void sysUpdateToolTip();
};

#endif //TRAYICON_H

#ifndef QTRAYICON_H
#define QTRAYICON_H

#ifndef QT_H
#include <qobject.h>
#include <qpixmap.h>
#endif // QT_H

class QPopupMenu;

class Q_EXPORT QTrayIcon : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
    Q_PROPERTY( QPixmap icon READ icon WRITE setIcon )

public:
    QTrayIcon( QObject *parent = 0, const char *name = 0 );
    QTrayIcon( const QPixmap &, const QString &, QPopupMenu *popup = 0, QObject *parent = 0, const char *name = 0 );
    ~QTrayIcon();

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;

    QPixmap		icon() const;
    QString		toolTip() const;

public slots:
    void		setIcon( const QPixmap &icon );
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

#endif //QTRAYICON_H

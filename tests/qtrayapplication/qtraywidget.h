#ifndef _TRAYAPP_H
#define _TRAYAPP_H

#include <qwidget.h>

class QPopupMenu;
class QPixmap;

class QTrayWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )

public:
    QTrayWidget( QWidget *parent = 0, const char *name = 0 );
    ~QTrayWidget();

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;

    QPixmap		icon() const;
    QString		toolTip() const;

public slots:
    void		show();
    void		hide();

    void		setIcon( const QPixmap &icon );
    void		setToolTip( const QString &tip );

signals:
    void		clicked( const QPoint& );
    void		doubleClicked( const QPoint& );

protected:
    void		mouseReleaseEvent( QMouseEvent *e );
    void		mouseDoubleClickEvent( QMouseEvent *e );

    void		closeEvent( QCloseEvent *e );

#if defined(Q_OS_WIN32)
    bool		winEvent( MSG * );
#endif

private:
    class QTrayWidgetPrivate;

    QTrayWidgetPrivate *d;
};

#endif

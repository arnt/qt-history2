/****************************************************************************
** $Id: $
**
** Definition of QActiveXControl
**
*****************************************************************************/

#ifndef QACTIVEX_H
#define QACTIVEX_H

#include <qwidget.h>


class QActiveXControl : public QWidget
{
    Q_OBJECT
public:
    QActiveXControl( const char *name=0 );
   ~QActiveXControl();

    bool	    isActive() const;

    virtual void    drawControl( QPainter *, const QRect & );

    virtual void    paintEvent( QPaintEvent * );
    virtual void    activateEvent( QEvent * );
    virtual void    deactivateEvent( QEvent * );

    static  void    initialize();
    static  void    terminate();

// These are semi-internal functions and should be called
// from the ATL control only.
	    void    setComControl( void * );
    virtual void    setActive( bool activate, HANDLE parentWindow );
    virtual void    drawControlAtl( void *atlDrawInfo );

public slots:
    void	    updateControl();

protected:
    bool	    event( QEvent * );

private:
    void	   *com_control;
    bool	    is_active;
    HANDLE	    old_parent;
    int		    old_style;
};


inline bool QActiveXControl::isActive() const
{
    return is_active;
}


#endif // QACTIVEX_H

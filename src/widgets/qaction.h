
#ifndef QT_H
#include <qobject.h>
#include <qiconset.h>
#include <qstring.h>
#endif // QT_H

class QActionPrivate;
class QActionGroupPrivate;

class Q_EXPORT QAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool toggleAction READ isToggleAction WRITE setToggleAction)
    Q_PROPERTY( bool on READ isOn WRITE setOn )
    Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled )
    Q_PROPERTY( QIconSet iconSet READ iconSet WRITE setIconSet )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QString menuText READ menuText WRITE setMenuText )
    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
    Q_PROPERTY( QString whatsThis READ whatsThis WRITE setWhatsThis )
    Q_PROPERTY( int accel READ accel WRITE setAccel )

public:
    QAction( QObject* parent, const char* name = 0, bool toggle = FALSE  );
    QAction( const QString& text, const QIconSet& icon, const QString& menuText, int accel, 
	     QObject* parent, const char* name = 0, bool toggle = FALSE );
    QAction( const QString& text, const QString& menuText, int accel, QObject* parent, 
	     const char* name = 0, bool toggle = FALSE );
    ~QAction();

    virtual void setIconSet( const QIconSet& );
    QIconSet iconSet() const;
    virtual void setText( const QString& );
    QString text() const;
    virtual void setMenuText( const QString& );
    QString menuText() const;
    virtual void setToolTip( const QString& );
    QString toolTip() const;
    virtual void setWhatsThis( const QString& );
    QString whatsThis() const;
    virtual void setAccel( int key );
    int accel() const;
    virtual void setToggleAction( bool );
    bool isToggleAction() const;
    virtual void setOn( bool );
    bool isOn() const;
    bool isEnabled() const;
    virtual bool addTo( QWidget* );
    virtual bool removeFrom( QWidget* );

public slots:
    virtual void setEnabled( bool );

signals:
    void activated();
    void toggled( bool );

private slots:
    void internalActivation();
    void toolButtonToggled( bool );
    void objectDestroyed();

private:
    void init(); 
    
    QActionPrivate* d;

};

class Q_EXPORT QActionGroup : public QAction
{
    Q_OBJECT
    Q_PROPERTY( bool exclusive READ isExclusive WRITE setExclusive )

public:
    QActionGroup( QWidget* parent, const char* name = 0, bool exclusive = TRUE );
    ~QActionGroup();
    void setExclusive( bool );
    bool isExclusive() const;
    void insert( QAction* );
    bool addTo( QWidget* );
    bool removeFrom( QWidget* );
    void setEnabled( bool );

signals:
    void selected( QAction* );

private slots:
    void childToggled( bool );
    void childDestroyed();

private:
    QActionGroupPrivate* d;

};

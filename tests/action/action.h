#ifndef ACTION_H
#define ACTION_H

#include <qaction.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qfontdatabase.h>

class QToggleAction : public QAction
{
    Q_OBJECT
public:
    QToggleAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    QToggleAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QToggleAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    QToggleAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QToggleAction( QObject* parent = 0, const char* name = 0 );
    
    int plug( QWidget* );

    virtual void setChecked( bool );
    bool isChecked();

protected slots:
    void slotActivated();
    
signals:
    void toggled( bool );
    
private:
    bool m_checked;
    bool m_lock;
};

class QSelectAction : public QAction
{
    Q_OBJECT
public:
    QSelectAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    QSelectAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QSelectAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    QSelectAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QSelectAction( QObject* parent = 0, const char* name = 0 );
    
    int plug( QWidget* );

    virtual void setItems( const QStringList& );
    QStringList items();
    QString currentText();
    int currentItem();
    virtual void setCurrentItem( int id );
    virtual void clear();
    
    QPopupMenu* popupMenu();
    
protected slots:
    virtual void slotActivated( int );
    
signals:
    void activated( int index );
    void activated( const QString& text );
    
private:
    bool m_lock;
    QStringList m_list;
    QPopupMenu* m_menu;
    int m_current;
};

class QFontAction : public QSelectAction
{
    Q_OBJECT
public:
    QFontAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    QFontAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QFontAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    QFontAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QFontAction( QObject* parent = 0, const char* name = 0 );
    
private:
    QFontDatabase m_fdb;
};

class QFontSizeAction : public QSelectAction
{
    Q_OBJECT
public:
    QFontSizeAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    QFontSizeAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QFontSizeAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    QFontSizeAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QFontSizeAction( QObject* parent = 0, const char* name = 0 );

    int plug( QWidget* );
    
    void setFontSize( int size );
    int fontSize();

protected slots:
    virtual void slotActivated( int );
    virtual void slotActivated( const QString& );

signals:
    void fontSizeChanged( int );
    
private:
    void init();
    
    bool m_lock;
};

#endif

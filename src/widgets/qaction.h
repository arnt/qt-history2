#ifndef QACTION_H
#define QACTION_H

#include "qdom.h"
#include "qobject.h"
#include "qiconset.h"
#include "qpixmap.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qwidget.h"
#include "qdialog.h"

class QToolBar;
class QMenuBar;
class QPopupMenu;
class QComboBox;
class QPoint;
class QIconView;
class QIconViewItem;

class QAction : public QObject
{
    Q_OBJECT
    Q_BUILDER( "A user action", "" )
public:
    QAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    QAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    QAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    QAction( QObject* parent = 0, const char* name = 0 );
    virtual ~QAction();

    virtual void update();

    void incref();
    bool decref();
    int count();

    virtual int plug( QWidget* );
    virtual void unplug( QWidget* );

    virtual bool isPlugged() const;

    QDomElement configuration( QDomDocument& doc, bool properties ) const;

    QWidget* container( int index );
    QWidget* representative( int index );
    int containerCount() const;

    // ##### todo setPixmap
    virtual QPixmap pixmap() const;

    virtual bool hasIconSet() const;
    virtual QString plainText() const;

    virtual QObject* component();
    virtual void setComponent( QObject* );

q_properties:
    virtual void setText( const QString& text );
    virtual QString text() const;

    virtual void setIconSet( const QIconSet& );
    virtual QIconSet iconSet() const;

    virtual void setWhatsThis( const QString& text );
    virtual QString whatsThis() const;

    virtual bool isEnabled() const;
    virtual void setEnabled( bool );

    virtual void setGroup( const QString& );
    virtual QString group() const;

    virtual void setAccel( int );
    virtual int accel() const;

protected slots:
    virtual void slotDestroyed();

protected:
    QToolBar* toolBar( int index );
    QPopupMenu* popupMenu( int index );
    int menuId( int index );
    void removeContainer( int index );
    int findContainer( QWidget* widget );

    void addContainer( QWidget* parent, int id );
    void addContainer( QWidget* parent, QWidget* representative );

signals:
    void activated();
    void enabled( bool );

protected slots:
    virtual void slotActivated();

private:
    QObject* m_component;
    int m_count;
    QString m_text;
    QString m_whatsThis;
    QString m_groupText;
    QPixmap m_pixmap;
    QIconSet m_iconSet;
    bool m_bIconSet;
    QString m_group;
    int m_accel;

    struct Container
    {
	Container() { m_container = 0; m_representative = 0; m_id = 0; }
	Container( const Container& s ) { m_container = s.m_container;
	                                  m_id = s.m_id; m_representative = s.m_representative; }
	QWidget* m_container;
	int m_id;
	QWidget* m_representative;
    };

    QValueList<Container> m_containers;
    bool m_enabled;
};

class QActionSeparator : public QAction
{
    Q_OBJECT
    Q_BUILDER( "", "" )
public:
    QActionSeparator( QObject* parent = 0, const char* name = 0 );
    ~QActionSeparator();

    virtual int plug( QWidget* );
    virtual void unplug( QWidget* );
};

class QActionMenu : public QAction
{
    Q_OBJECT
    Q_BUILDER( "A user action menu", "" )
public:
    QActionMenu( const QString& text, QObject* parent = 0, const char* name = 0 );
    QActionMenu( QObject* parent = 0, const char* name = 0 );
    virtual ~QActionMenu();

    virtual int plug( QWidget* );
    virtual void unplug( QWidget* );

    virtual void insert( QAction* );
    virtual void remove( QAction* );

    QPopupMenu* popupMenu();

    virtual bool setConfiguration( const QDomElement& element );

private:
    QPopupMenu* m_popup;
};

class QActionCollection : public QObject
{
    Q_OBJECT
    Q_BUILDER( "A collection for actions", "" )
public:
    QActionCollection( QObject* parent = 0, const char* name = 0 );
    ~QActionCollection();

    virtual void insert( QAction* );
    virtual void remove( QAction* );
    virtual QAction* take( QAction* );

    virtual QAction* action( int index );
    virtual uint count() const;
    virtual QAction* action( const char* name, const char* classname = 0, QObject* component = 0 );

    virtual QStringList groups() const;
    virtual QValueList<QAction*> actions( const QString& group );
    virtual QValueList<QAction*> actions();

    bool setConfiguration( const QDomElement& element );
    QDomElement configuration( QDomDocument& doc, bool properties ) const;

signals:
    void inserted( QAction* );
    void removed( QAction* );

protected:
    void childEvent( QChildEvent* );

private:
    QList<QAction> m_actions;
};

class QActionWidget : public QWidget
{
    Q_OBJECT
    Q_BUILDER( "A widget for selecting actions", "" )
public:
    QActionWidget( QWidget* parent = 0, const char* name = 0 );
    QActionWidget( QActionCollection*, QWidget* parent = 0, const char* name = 0 );
    ~QActionWidget();

    virtual QAction* currentAction();

    virtual void clearSelection();
    virtual QAction* selectedAction();

    virtual QActionCollection* collection();
    virtual void setCollection( QActionCollection* );

    virtual void addGroup( const QString& group );
    virtual QString currentGroup() const;
    virtual void setCurrentGroup( const QString& grp, bool update = FALSE );

    void updateAction( QAction* );

signals:
    void rightButtonPressed( QAction*, const QPoint& );
    void selectionChanged( QAction* );

protected slots:
    void slotDropped( QDropEvent* );
    void insertAction( QAction* );
    void removeAction( QAction* );

private slots:
    void rightButtonPressed( QIconViewItem*, const QPoint& );
    void showGroup( const QString& grp );
    void selectionChanged();

private:
    void init();

private:
    QIconView* m_icons;
    QComboBox* m_group;
    QActionCollection* m_collection;
};

class QActionDialog : public QDialog
{
    Q_OBJECT
public:
    QActionDialog( QActionCollection*, QWidget* parent = 0, const char* name = 0, bool modal = FALSE );
    ~QActionDialog();

    QActionWidget* actionWidget();

private:
    QActionWidget* m_widget;
};

#endif

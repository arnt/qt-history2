#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#include <qaccessible.h>

class QButton;
class QScrollView;
class QHeader;
class QSpinWidget;
class QScrollBar;
class QSlider;
class QListBox;
class QListView;
class QIconView;
class QTextEdit;
class QTabBar;
class QWidgetStack;

QString buddyString( QWidget * );
QString stripAmp( const QString& );
QString hotKey( const QString& );

class QAccessibleWidget : public QAccessibleObject
{
public:
    QAccessibleWidget( QObject *o, Role r = Client, QString name = QString::null, 
	QString description = QString::null, QString value = QString::null, 
	QString help = QString::null, QString defAction = QString::null,
	QString accelerator = QString::null, State s = Normal );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;
    QRESULT	queryParent( QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;

protected:
    QWidget *widget() const;

private:
    Role role_;
    State state_;
    QString name_;
    QString description_;
    QString value_;
    QString help_;
    QString defAction_;
    QString accelerator_;
};

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack( QObject *o );

    int		controlAt( int x, int y ) const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

protected:
    QWidgetStack *widgetStack() const;
};

class QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton( QObject *o, Role r, QString description = QString::null,
	QString help = QString::null );

    QString	text( Text t, int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );

protected:
    QButton *button() const;
};

class QAccessibleRangeControl : public QAccessibleWidget
{
public:
    QAccessibleRangeControl( QObject *o, Role role, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    QString	text( Text t, int control ) const;
};

class QAccessibleSpinWidget : public QAccessibleRangeControl
{
public:
    QAccessibleSpinWidget( QObject *o );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
};

class QAccessibleScrollBar : public QAccessibleRangeControl
{
public:
    QAccessibleScrollBar( QObject *o, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;

    bool	doDefaultAction( int control );

protected:
    QScrollBar *scrollBar() const;
};

class QAccessibleSlider : public QAccessibleRangeControl
{
public:
    QAccessibleSlider( QObject *o, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;

    bool	doDefaultAction( int control );

protected:
    QSlider *slider() const;
};

class QAccessibleText : public QAccessibleWidget
{
public:
    QAccessibleText( QObject *o, Role role, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    QString	text( Text t, int control ) const;
    State	state( int control ) const;
};

class QAccessibleDisplay : public QAccessibleWidget
{
public:
    QAccessibleDisplay( QObject *o, Role role, QString description = QString::null, 
	QString value = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
};

class QAccessibleHeader : public QAccessibleWidget
{
public:
    QAccessibleHeader( QObject *o, QString description = QString::null, 
	QString value = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;

    Role role( int control ) const;
    State state( int control ) const;

protected:
    QHeader *header() const;
};

class QAccessibleTabBar : public QAccessibleWidget
{
public:
    QAccessibleTabBar( QObject *o, QString description = QString::null, 
	QString value = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;

    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;    

protected:
    QTabBar *tabBar() const;
};

class QAccessibleScrollView : public QAccessibleWidget
{
public:
    QAccessibleScrollView( QObject *o, Role role, QString name = QString::null,
	QString description = QString::null, QString value = QString::null, 
	QString help = QString::null, QString defAction = QString::null, 
	QString accelerator = QString::null );

    QString	text( Text t, int control ) const;

    virtual int itemHitTest( int x, int y ) const;
    virtual QRect itemLocation( int item ) const;
    virtual int itemCount() const;
};

class QAccessibleViewport : public QAccessibleWidget
{
public:
    QAccessibleViewport( QObject *o, QObject *sv );

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;

protected:
    QAccessibleScrollView *scrollView() const;
    QScrollView *scrollview;
};

class QAccessibleListBox : public QAccessibleScrollView
{
public:
    QAccessibleListBox( QObject *o );

    int		itemHitTest( int x, int y ) const;
    QRect	itemLocation( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;

protected:
    QListBox *listBox() const;
};

class QAccessibleListView : public QAccessibleScrollView
{
public:
    QAccessibleListView( QObject *o );

    int		itemHitTest( int x, int y ) const;
    QRect	itemLocation( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;

protected:
    QListView *listView() const;
};

class QAccessibleIconView : public QAccessibleScrollView
{
public:
    QAccessibleIconView( QObject *o );

    int		itemHitTest( int x, int y ) const;
    QRect	itemLocation( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;

protected:
    QIconView *iconView() const;
};

class QAccessibleTextEdit : public QAccessibleScrollView
{
public:
    QAccessibleTextEdit( QObject *o );

    int		itemHitTest( int x, int y ) const;
    QRect	itemLocation( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;

protected:
    QTextEdit *textEdit() const;
};

#endif // Q_ACESSIBLEWIDGET_H

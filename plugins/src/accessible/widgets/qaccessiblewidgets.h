#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <qaccessiblewidget.h>

class QButton;
class QScrollView;
class QHeader;
class QSpinWidget;
class QScrollBar;
class QSlider;
class QListBox;
class QListView;
class QTextEdit;
class QTabBar;
class QComboBox;
class QTitleBar;
class QWidgetStack;

#ifndef QT_NO_ICONVIEW
class QIconView;
#endif


QString buddyString( QWidget * );
QString stripAmp( const QString& );
QString hotKey( const QString& );

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack( QObject *o );

    int		childAt( int x, int y ) const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

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

    bool	doAction(int action, int control);

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

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doAction(int action, int control);
};

class QAccessibleScrollBar : public QAccessibleRangeControl
{
public:
    QAccessibleScrollBar( QObject *o, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;

    bool	doAction(int action, int control);

protected:
    QScrollBar *scrollBar() const;
};

class QAccessibleSlider : public QAccessibleRangeControl
{
public:
    QAccessibleSlider( QObject *o, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;

    bool	doAction(int action, int control);

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

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

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

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;

    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doAction(int action, int control);
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QVector<int> selection() const;    

protected:
    QTabBar *tabBar() const;
};

class QAccessibleComboBox : public QAccessibleWidget
{
public:
    QAccessibleComboBox( QObject *o );

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;

    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doAction(int action, int control);

protected:
    QComboBox *comboBox() const;
};

class QAccessibleTitleBar : public QAccessibleWidget
{
public:
    QAccessibleTitleBar( QObject *o );

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doAction(int action, int control);

protected:
    QTitleBar *titleBar() const;
};

class QAccessibleScrollView : public QAccessibleWidget
{
public:
    QAccessibleScrollView( QObject *o, Role role, QString name = QString::null,
	QString description = QString::null, QString value = QString::null, 
	QString help = QString::null, QString defAction = QString::null, 
	QString accelerator = QString::null );

    QString	text( Text t, int control ) const;

    virtual int itemAt( int x, int y ) const;
    virtual QRect itemRect( int item ) const;
    virtual int itemCount() const;
};

class QAccessibleViewport : public QAccessibleWidget
{
public:
    QAccessibleViewport( QObject *o, QObject *sv );

    int		childAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doAction(int action, int control);
    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QAccessibleScrollView *scrollView() const;
    QScrollView *scrollview;
};

class QAccessibleListBox : public QAccessibleScrollView
{
public:
    QAccessibleListBox( QObject *o );

    int		itemAt( int x, int y ) const;
    QRect	itemRect( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QListBox *listBox() const;
};

class QAccessibleListView : public QAccessibleScrollView
{
public:
    QAccessibleListView( QObject *o );

    int		itemAt( int x, int y ) const;
    QRect	itemRect( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QListView *listView() const;
};

#ifndef QT_NO_ICONVIEW
class QAccessibleIconView : public QAccessibleScrollView
{
public:
    QAccessibleIconView( QObject *o );

    int		itemAt( int x, int y ) const;
    QRect	itemRect( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QIconView *iconView() const;
};
#endif

class QAccessibleTextEdit : public QAccessibleScrollView
{
public:
    QAccessibleTextEdit( QObject *o );

    int		itemAt( int x, int y ) const;
    QRect	itemRect( int item ) const;
    int		itemCount() const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;

protected:
    QTextEdit *textEdit() const;
};

#endif // Q_ACESSIBLEWIDGETS_H

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


QString buddyString(QWidget *);
QString stripAmp(const QString&);
QString hotKey(const QString&);

class QAccessibleComplexWidget : public QAccessibleWidget
{
public:
    QAccessibleComplexWidget( QWidget *o, Role r = Client, QString name = QString(), 
	QString description = QString(), QString value = QString(), 
	QString help = QString(), int defAction = SetFocus, QString defActionName = QString(),
	QString accelerator = QString(), State s = Normal );

    int		childAt(int x, int y) const;
};

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack(QWidget *o);

    int		childAt(int x, int y) const;

protected:
    QWidgetStack *widgetStack() const;
};

class QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton(QWidget *o, Role r, QString description = QString(),
	QString help = QString());

    QString	text(Text t, int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QButton *button() const;
};

class QAccessibleRangeControl : public QAccessibleComplexWidget
{
public:
    QAccessibleRangeControl(QWidget *o, Role role, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    QString	text(Text t, int child) const;
};

class QAccessibleSpinWidget : public QAccessibleRangeControl
{
public:
    QAccessibleSpinWidget(QWidget *o);

    int		childCount() const;
    QRect	rect(int child) const;

    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);
};

class QAccessibleScrollBar : public QAccessibleRangeControl
{
public:
    QAccessibleScrollBar(QWidget *o, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;
    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;

    bool	doAction(int action, int child);

protected:
    QScrollBar *scrollBar() const;
};

class QAccessibleSlider : public QAccessibleRangeControl
{
public:
    QAccessibleSlider(QWidget *o, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;
    int		relationTo(int child, const QAccessibleInterface *other, int otherChild);

    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;

    bool	doAction(int action, int child);

protected:
    QSlider *slider() const;
};

class QAccessibleText : public QAccessibleWidget
{
public:
    QAccessibleText(QWidget *o, Role role, QString name = QString(), 
	QString description = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    QString	text(Text t, int child) const;
    State	state(int child) const;
};

class QAccessibleDisplay : public QAccessibleWidget
{
public:
    QAccessibleDisplay(QWidget *o, Role role, QString description = QString(), 
	QString value = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    QString	text(Text t, int child) const;
    Role	role(int child) const;
};

class QAccessibleHeader : public QAccessibleComplexWidget
{
public:
    QAccessibleHeader(QWidget *o, QString description = QString(), 
	QString value = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;
    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

protected:
    QHeader *header() const;
};

class QAccessibleTabBar : public QAccessibleComplexWidget
{
public:
    QAccessibleTabBar(QWidget *o, QString description = QString(), 
	QString value = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;
    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);
    bool	setSelected(int child, bool on, bool extend);
    QVector<int> selection() const;

protected:
    QTabBar *tabBar() const;
};

class QAccessibleComboBox : public QAccessibleComplexWidget
{
public:
    QAccessibleComboBox(QWidget *o);

    int		childCount() const;
    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString	text(Text t, int child) const;
    QRect	rect(int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QComboBox *comboBox() const;
};

class QAccessibleTitleBar : public QAccessibleComplexWidget
{
public:
    QAccessibleTitleBar(QWidget *o);

    int		childCount() const;

    QString	text(Text t, int child) const;
    QRect	rect(int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QTitleBar *titleBar() const;
};

class QAccessibleScrollView : public QAccessibleWidget
{
public:
    QAccessibleScrollView(QWidget *o, Role role, QString name = QString(),
	QString description = QString(), QString value = QString(), 
	QString help = QString(), QString defAction = QString(), 
	QString accelerator = QString());

    QString	text(Text t, int child) const;

    virtual int itemAt(int x, int y) const;
    virtual QRect itemRect(int item) const;
    virtual int itemCount() const;
};

class QAccessibleViewport : public QAccessibleWidget
{
public:
    QAccessibleViewport(QWidget *o,QWidget *sv);

    int		childAt(int x, int y) const;
    int		childCount() const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);
    bool	setSelected(int child, bool on, bool extend);
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QAccessibleScrollView *scrollView() const;
    QScrollView *scrollview;
};

class QAccessibleListBox : public QAccessibleScrollView
{
public:
    QAccessibleListBox(QWidget *o);

    int		itemAt(int x, int y) const;
    QRect	itemRect(int item) const;
    int		itemCount() const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	setSelected(int child, bool on, bool extend);
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QListBox *listBox() const;
};

class QAccessibleListView : public QAccessibleScrollView
{
public:
    QAccessibleListView(QWidget *o);

    int		itemAt(int x, int y) const;
    QRect	itemRect(int item) const;
    int		itemCount() const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	setSelected(int child, bool on, bool extend);
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QListView *listView() const;
};

#ifndef QT_NO_ICONVIEW
class QAccessibleIconView : public QAccessibleScrollView
{
public:
    QAccessibleIconView(QWidget *o);

    int		itemAt(int x, int y) const;
    QRect	itemRect(int item) const;
    int		itemCount() const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	setSelected(int child, bool on, bool extend);
    void	clearSelection();
    QVector<int> selection() const;

protected:
    QIconView *iconView() const;
};
#endif

class QAccessibleTextEdit : public QAccessibleScrollView
{
public:
    QAccessibleTextEdit(QWidget *o);

    int		itemAt(int x, int y) const;
    QRect	itemRect(int item) const;
    int		itemCount() const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;

protected:
    QTextEdit *textEdit() const;
};

#endif // Q_ACESSIBLEWIDGETS_H

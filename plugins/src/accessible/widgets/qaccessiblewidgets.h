#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <qaccessiblewidget.h>

class QScrollView;
class QHeader;
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

class QAccessibleWidgetStack : public QAccessibleWidget
{
public:
    QAccessibleWidgetStack(QWidget *o);

    int		childAt(int x, int y) const;

protected:
    QWidgetStack *widgetStack() const;
};

class QAccessibleHeader : public QAccessibleWidget
{
public:
    QAccessibleHeader(QWidget *o, QString description = QString(), 
	QString value = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;

    QRect	rect(int child) const;
    QString	text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

protected:
    QHeader *header() const;
};

class QAccessibleTabBar : public QAccessibleWidget
{
public:
    QAccessibleTabBar(QWidget *o, QString description = QString(), 
	QString value = QString(), QString help = QString(), 
	QString defAction = QString(), QString accelerator = QString());

    int		childCount() const;

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

class QAccessibleComboBox : public QAccessibleWidget
{
public:
    QAccessibleComboBox(QWidget *o);

    int		childCount() const;
    int		childAt(int x, int y) const;
    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString	text(Text t, int child) const;
    QRect	rect(int child) const;
    Role	role(int child) const;
    State	state(int child) const;

    bool	doAction(int action, int child);

protected:
    QComboBox *comboBox() const;
};

class QAccessibleTitleBar : public QAccessibleWidget
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

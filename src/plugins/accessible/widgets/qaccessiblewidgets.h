#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <qaccessiblewidget.h>

class QScrollView;
class QListBox;
class QListView;
class QTextEdit;

#ifndef QT_NO_ICONVIEW
class QIconView;
#endif

class QAccessibleScrollView : public QAccessibleWidget
{
public:
    QAccessibleScrollView(QWidget *w, Role role);

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
    int		state(int child) const;

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
    int		state(int child) const;

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
    int		state(int child) const;

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
    int		state(int child) const;

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

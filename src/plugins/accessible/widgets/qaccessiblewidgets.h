#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <qaccessiblewidget.h>

class QListBox;

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
    ~QAccessibleViewport();

    int                childAt(int x, int y) const;
    int                childCount() const;

    QRect        rect(int child) const;
    QString        text(Text t, int child) const;
    Role        role(int child) const;
    int                state(int child) const;

    bool        doAction(int action, int child, const QVariantList &params);
    bool        setSelected(int child, bool on, bool extend);
    void        clearSelection();
    QVector<int> selection() const;

protected:
    QAccessibleScrollView *scrollview;
};

#endif // Q_ACESSIBLEWIDGETS_H

#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QWidgetItem>

class FlowLayout : public QLayout
{
public:
    FlowLayout(QWidget *parent, int margin = 0, int spacing = -1);
    FlowLayout(QLayout *parent, int spacing);
    FlowLayout(int spacing);
    ~FlowLayout();

    void addItem(QLayoutItem *item);
    QSizePolicy::ExpandData expanding() const;
    bool hasHeightForWidth() const;
    int heightForWidth(int) const;
    QLayoutItem *itemAt(int index) const;
    QSize minimumSize() const;
    void setGeometry(const QRect &rect);
    QSize sizeHint() const;
    QLayoutItem *takeAt(int index);

private:
    int doLayout(const QRect &rect, bool testOnly) const;

    QList<QLayoutItem *> itemList;
};

#endif

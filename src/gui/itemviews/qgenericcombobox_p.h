#ifndef QGENERICCOMBOBOX_P_H
#define QGENERICCOMBOBOX_P_H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qlineedit.h>
#include <qgenericcombobox.h>
#include <qbasictimer.h>
#include <qabstractslider.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <private/qwidget_p.h>
#endif // QT_H

class Scroller : public QWidget
{
    Q_OBJECT

public:
    Scroller(QAbstractSlider::SliderAction action, QWidget *parent)
        : QWidget(parent), sliderAction(action) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }
    QSize sizeHint() const {
        return QSize(20, style().pixelMetric(QStyle::PM_MenuScrollerHeight));
    }

protected:
    void enterEvent(QEvent *) {
        timer.start(100, this);
    }
    void leaveEvent(QEvent *) {
        timer.stop();
    }
    void timerEvent(QTimerEvent *e) {
        if (e->timerId() == timer.timerId())
            emit doScroll(sliderAction);
    }
    void hideEvent(QHideEvent *) {
        timer.stop();
    }
    void paintEvent(QEvent *e) {
        QPainter p(this);
        Q4StyleOptionMenuItem menuOpt(0);
        menuOpt.palette = palette();
        menuOpt.state = QStyle::Style_Default;
        menuOpt.checkState = Q4StyleOptionMenuItem::NotCheckable;
        menuOpt.menuRect = rect();
        menuOpt.maxIconWidth = 0;
        menuOpt.tabWidth = 0;
        menuOpt.menuItemType = Q4StyleOptionMenuItem::Scroller;
        if (sliderAction == QAbstractSlider::SliderSingleStepAdd)
            menuOpt.state = QStyle::Style_Down;
        menuOpt.rect = rect();
        style().drawControl(QStyle::CE_MenuScroller, &menuOpt, &p);
    }

signals:
    void doScroll(int action);

private:
    QAbstractSlider::SliderAction sliderAction;
    QBasicTimer timer;
};

class ListViewContainer : public QFrame
{
    Q_OBJECT

public:
    ListViewContainer(QGenericListView *listView, QWidget *parent = 0);
    QGenericListView *listView() const;

public slots:
    void scrollListView(int action);
    void updateScrollers();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);

signals:
    void itemSelected(const QModelIndex &);

private:
    QGenericListView *list;
    Scroller *top;
    Scroller *bottom;
};

class QGenericComboBoxPrivate: public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGenericComboBox)
public:
    QGenericComboBoxPrivate()
        : QWidgetPrivate(),
          model(0),
          lineEdit(0),
          delegate(0),
          container(0),
          insertionPolicy(QGenericComboBox::AtBottom),
          autoCompletion(true),
          duplicatesEnabled(false),
          sizeLimit(10),
          ignoreMousePressEvent(false),
          skipCompletion(false) {}
    ~QGenericComboBoxPrivate() {}
    void init();
    void updateLineEditGeometry();
    void returnPressed();
    void complete();
    void itemSelected(const QModelIndex &item);
    bool contains(const QString &text, int role);

    QAbstractItemModel *model;
    QLineEdit *lineEdit;
    QAbstractItemDelegate *delegate;
    ListViewContainer *container;
    QGenericComboBox::InsertionPolicy insertionPolicy;
    bool autoCompletion;
    bool duplicatesEnabled;
    int sizeLimit;
    bool ignoreMousePressEvent;
    bool skipCompletion;
    mutable QSize sizeHint;
    QPersistentModelIndex currentItem;
    QPersistentModelIndex root;
};

#endif //QGENERICCOMBOBOX_P_H

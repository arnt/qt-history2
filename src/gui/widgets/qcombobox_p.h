/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOMBOBOX_P_H
#define QCOMBOBOX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qwidget_p.h>
#include <qabstractslider.h>
#include <qapplication.h>
#include <qbasictimer.h>
#include <qcombobox.h>
#include <qhash.h>
#include <qitemdelegate.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qpair.h>
#include <qstyle.h>
#include <qstyleoption.h>

#include <limits.h>

class Scroller : public QWidget
{
    Q_OBJECT

public:
    Scroller(QAbstractSlider::SliderAction action, QWidget *parent)
        : QWidget(parent), sliderAction(action) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }
    QSize sizeHint() const {
        return QSize(20, style()->pixelMetric(QStyle::PM_MenuScrollerHeight));
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
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        QStyleOptionMenuItem menuOpt;
        menuOpt.palette = palette();
        menuOpt.state = QStyle::State_None;
        menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
        menuOpt.menuRect = rect();
        menuOpt.rect = rect();
        menuOpt.maxIconWidth = 0;
        menuOpt.tabWidth = 0;
        menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
        if (sliderAction == QAbstractSlider::SliderSingleStepAdd)
            menuOpt.state = QStyle::State_Down;
        p.eraseRect(rect());
        style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p);
    }

signals:
    void doScroll(int action);

private:
    QAbstractSlider::SliderAction sliderAction;
    QBasicTimer timer;
};

class ItemViewContainer : public QFrame
{
    Q_OBJECT

public:
    ItemViewContainer(QAbstractItemView *itemView, QComboBox *parent);
    QAbstractItemView *itemView() const;
    void setItemView(QAbstractItemView *itemView);
    int spacing() const;

public slots:
    void scrollItemView(int action);
    void updateScrollers();
    void setCurrentIndex(const QModelIndex &index);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void hideEvent(QHideEvent *e);
    QStyleOptionComboBox comboStyleOption() const;

signals:
    void itemSelected(const QModelIndex &);
    void containerDisappearing();

private:
    QComboBox *combo;
    QAbstractItemView *view;
    Scroller *top;
    Scroller *bottom;
};

class MenuDelegate : public QAbstractItemDelegate
{
public:
    MenuDelegate(QObject *parent, QComboBox *cmb) : QAbstractItemDelegate(parent), mCombo(cmb), pal(QApplication::palette("QMenu")) {}

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        painter->eraseRect(option.rect);
        mCombo->style()->drawControl(QStyle::CE_MenuItem, &opt, painter, 0);
    }
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        QVariant value = index.model()->data(index, QAbstractItemModel::FontRole);
        QFont fnt = value.isValid() ? value.toFont() : option.font;
        return mCombo->style()->sizeFromContents(
            QStyle::CT_MenuItem, &opt, option.rect.size(), 0);
    }

private:
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const;
    QComboBox *mCombo;
    QPalette pal;
};

class QComboBoxPrivate: public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QComboBox)
public:
    QComboBoxPrivate()
        : QWidgetPrivate(),
          model(0),
          lineEdit(0),
          container(0),
          insertPolicy(QComboBox::InsertAtBottom),
          autoCompletion(true),
          duplicatesEnabled(false),
          autoHide(true),
          arrowDown(false),
          maxVisibleItems(10),
          maxCount(INT_MAX),
          skipCompletion(false) {}
    ~QComboBoxPrivate() {}
    void init();
    QStyleOptionComboBox getStyleOption() const;
    void updateLineEditGeometry();
    void returnPressed();
    void complete();
    void itemSelected(const QModelIndex &item);
    bool contains(const QString &text, int role);
    void emitActivated(const QModelIndex&);
    void emitHighlighted(const QModelIndex&);
    void resetButton();

    QAbstractItemModel *model;
    QLineEdit *lineEdit;
    ItemViewContainer *container;
    QComboBox::InsertPolicy insertPolicy;
    bool autoCompletion;
    bool duplicatesEnabled;
    bool autoHide;
    bool arrowDown;
    int maxVisibleItems;
    int maxCount;
    bool skipCompletion;
    mutable QSize sizeHint;
    QPersistentModelIndex currentIndex;
    QPersistentModelIndex root;
};

#endif // QCOMBOBOX_P_H

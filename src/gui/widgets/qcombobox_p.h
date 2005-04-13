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
#include <qtimer.h>

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
            menuOpt.state = QStyle::State_DownArrow;
        p.eraseRect(rect());
        style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p);
    }

signals:
    void doScroll(int action);

private:
    QAbstractSlider::SliderAction sliderAction;
    QBasicTimer timer;
};

class QComboBoxPrivateContainer : public QFrame
{
    Q_OBJECT

public:
    QComboBoxPrivateContainer(QAbstractItemView *itemView, QComboBox *parent);
    QAbstractItemView *itemView() const;
    void setItemView(QAbstractItemView *itemView);
    int spacing() const;
    QTimer blockMouseReleaseTimer;

public slots:
    void scrollItemView(int action);
    void updateScrollers();
    void setCurrentIndex(const QModelIndex &index);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void hideEvent(QHideEvent *e);
    QStyleOptionComboBox comboStyleOption() const;

signals:
    void itemSelected(const QModelIndex &);
    void resetButton();

private:
    QComboBox *combo;
    QAbstractItemView *view;
    Scroller *top;
    Scroller *bottom;
};

class QComboMenuDelegate : public QAbstractItemDelegate
{
public:
    QComboMenuDelegate(QObject *parent, QComboBox *cmb) : QAbstractItemDelegate(parent), mCombo(cmb), pal(QApplication::palette("QMenu")) {}

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
        QVariant value = index.model()->data(index, Qt::FontRole);
        QFont fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;
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
    QComboBoxPrivate();
    ~QComboBoxPrivate() {}
    void init();
    QComboBoxPrivateContainer* viewContainer();
    QStyleOptionComboBox getStyleOption() const;
    void updateLineEditGeometry();
    void returnPressed();
    void complete();
    void itemSelected(const QModelIndex &item);
    bool contains(const QString &text, int role);
    void emitActivated(const QModelIndex&);
    void emitHighlighted(const QModelIndex&);
    void resetButton();
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void rowsInserted(const QModelIndex & parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
    void updateArrow(QStyle::StateFlag state);
    bool updateHoverControl(const QPoint &pos);
    QStyle::SubControl newHoverControl(const QPoint &pos);

    QAbstractItemModel *model;
    QLineEdit *lineEdit;
    QComboBoxPrivateContainer *container;
    QComboBox::InsertPolicy insertPolicy;
    QComboBox::SizeAdjustPolicy sizeAdjustPolicy;
    int minimumContentsLength;
    QSize iconSize;
    uint shownOnce : 1;
    uint autoCompletion : 1;
    uint duplicatesEnabled : 1;
    uint skipCompletion : 1;
    uint frame : 1;
    uint padding : 26;
    int maxVisibleItems;
    int maxCount;
    int modelColumn;
    mutable QSize sizeHint;
    QStyle::StateFlag arrowState;
    QStyle::SubControl hoverControl;
    QRect hoverRect;
    QPersistentModelIndex currentIndex;
    QPersistentModelIndex root;
};

#endif // QCOMBOBOX_P_H

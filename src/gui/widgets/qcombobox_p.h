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

#ifndef QT_H
#include <qgenericlistview.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbasictimer.h>
#include <qabstractslider.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <private/qwidget_p.h>
#include <limits.h>
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
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        QStyleOptionMenuItem menuOpt(0);
        menuOpt.palette = palette();
        menuOpt.state = QStyle::Style_Default;
        menuOpt.checkState = QStyleOptionMenuItem::NotCheckable;
        menuOpt.menuRect = rect();
        menuOpt.rect = rect();
        menuOpt.maxIconWidth = 0;
        menuOpt.tabWidth = 0;
        menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
        if (sliderAction == QAbstractSlider::SliderSingleStepAdd)
            menuOpt.state = QStyle::Style_Down;
        p.eraseRect(rect());
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
    bool ignoreNextMousePress();

public slots:
    void scrollListView(int action);
    void updateScrollers();
    void setCurrentItem(const QModelIndex &index, int bstate);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void hideEvent(QHideEvent *e);

signals:
    void itemSelected(const QModelIndex &);
    void containerDisappearing();

private:
    bool ignoreMousePress;
    QGenericListView *list;
    Scroller *top;
    Scroller *bottom;
};

class MenuDelegate : public QItemDelegate
{
public:
    MenuDelegate(QObject *parent) : QItemDelegate(parent) {}

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               QAbstractItemModel *model, const QModelIndex &index) const {
        QStyleOptionMenuItem opt = getStyleOption(option, model, index);
        painter->eraseRect(option.rect);
        QApplication::style().drawControl(QStyle::CE_MenuItem, &opt, painter, 0);
    }
    QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                   QAbstractItemModel *model, const QModelIndex &index) const {
        QStyleOptionMenuItem opt = getStyleOption(option, model, index);
        return QApplication::style().sizeFromContents(
            QStyle::CT_MenuItem, &opt, option.rect.size(), fontMetrics, 0);
    }

private:
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,
                                        QAbstractItemModel *model,
                                        const QModelIndex &index) const {
        QStyleOptionMenuItem opt(0);
        opt.palette = option.palette;
        opt.state = QStyle::Style_Default;
        if (option.state & QStyle::Style_Enabled)
            opt.state |= QStyle::Style_Enabled;
        else
            opt.palette.setCurrentColorGroup(QPalette::Disabled);
        opt.state |= QStyle::Style_ButtonDefault;
        if (option.state & QStyle::Style_Selected) {
            opt.state |= QStyle::Style_Active;
            opt.checkState = QStyleOptionMenuItem::Checked;
        } else {
            opt.checkState = QStyleOptionMenuItem::Unchecked;
        }
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.icon = model->data(index, QAbstractItemModel::Role_Decoration).toIconSet();
        opt.text = model->data(index, QAbstractItemModel::Role_Display).toString();
        opt.tabWidth = 0;
        opt.maxIconWidth = 0;
        if (!opt.icon.isNull())
            opt.maxIconWidth = opt.icon.pixmap(QIconSet::Small, QIconSet::Normal).width() + 4;
        opt.menuRect = option.rect;
        opt.rect = option.rect;
        return opt;
    }
};

class ComboModel : public QAbstractItemModel
{
public:
    QVariant data(const QModelIndex &index, int role = Role_Display) const {
        if ((role == Role_Display || role == Role_Edit || role == Role_Decoration)
            && index.type() == QModelIndex::View  && index.isValid()
            && index.row() < rowCount(parent(index))
            && index.column() < columnCount(parent(index))) {
            if (role == Role_Display || role == Role_Edit)
                return list.at(index.row()).first;
            if (role == Role_Decoration)
                return list.at(index.row()).second;
        }
        return QVariant::Invalid;
    }
    int columnCount(const QModelIndex &) const {
        return 1;
    }
    int rowCount(const  QModelIndex &) const {
        return list.count();
    }
    bool setData(const QModelIndex &index, int role, const QVariant &value) {
        if (role == Role_Display || role == Role_Edit) {
            list[index.row()].first = value.toString();
            emit dataChanged(index, index);
            return true;
        } else if (role == Role_Decoration) {
            list[index.row()].second = value.toIconSet();
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }
    bool isEditable(const QModelIndex &) const {
        return true;
    }
    bool insertRows(int row, const QModelIndex &parent, int count) {
        // this model only allows a 1D list
        if (parent.isValid() || count < 1 || row < 0 || row > rowCount(QModelIndex()))
            return false;

        QPair<QString, QIconSet> emptyPair;
        while (count--)
            list.insert(row, emptyPair);
        emit rowsInserted(parent, row, row+count-1);
        return true;
    }
    bool removeRows(int row, const QModelIndex &parent, int count) {
        // this model only allows a 1D list
        if (parent.isValid() || count < 1 || row < 0 || row+count > rowCount(QModelIndex()))
            return false;

        emit rowsRemoved(parent, row, row+count-1);
        for (int i=row+count-1; i >= row; --i)
            list.removeAt(i);
        return true;
    }

private:
    QList<QPair<QString,QIconSet> > list;
};

class QComboBoxPrivate: public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QComboBox)
public:
    QComboBoxPrivate()
        : QWidgetPrivate(),
          model(0),
          lineEdit(0),
          delegate(0),
          container(0),
          insertionPolicy(QComboBox::AtBottom),
          autoCompletion(true),
          duplicatesEnabled(false),
          arrowDown(false),
          sizeLimit(10),
          maxCount(INT_MAX),
          ignoreMousePressEvent(false),
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
    QAbstractItemDelegate *delegate;
    ListViewContainer *container;
    QComboBox::InsertionPolicy insertionPolicy;
    bool autoCompletion;
    bool duplicatesEnabled;
    bool arrowDown;
    int sizeLimit;
    int maxCount;
    bool ignoreMousePressEvent;
    bool skipCompletion;
    mutable QSize sizeHint;
    QPersistentModelIndex currentItem;
    QPersistentModelIndex root;
};

#endif //QCOMBOBOX_P_H

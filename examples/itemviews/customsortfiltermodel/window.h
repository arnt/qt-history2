/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_DECLARE_CLASS(QAbstractItemModel)
QT_DECLARE_CLASS(QCheckBox)
QT_DECLARE_CLASS(QComboBox)
QT_DECLARE_CLASS(QDateEdit)
QT_DECLARE_CLASS(QGroupBox)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QTreeView)
class MySortFilterProxyModel;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

    void setSourceModel(QAbstractItemModel *model);

private slots:
    void textFilterChanged();
    void dateFilterChanged();

private:
    MySortFilterProxyModel *proxyModel;

    QGroupBox *sourceGroupBox;
    QGroupBox *proxyGroupBox;
    QTreeView *sourceView;
    QTreeView *proxyView;
    QCheckBox *filterCaseSensitivityCheckBox;
    QLabel *filterPatternLabel;
    QLabel *fromLabel;
    QLabel *toLabel;
    QLineEdit *filterPatternLineEdit;
    QComboBox *filterSyntaxComboBox;
    QDateEdit *fromDateEdit;
    QDateEdit *toDateEdit;
};

#endif

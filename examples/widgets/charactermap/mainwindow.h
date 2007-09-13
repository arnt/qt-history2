/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_DECLARE_CLASS(QClipboard)
QT_DECLARE_CLASS(QComboBox)
QT_DECLARE_CLASS(QFontComboBox)
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QScrollArea)
QT_DECLARE_CLASS(QCheckBox)
class CharacterWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void findStyles(const QFont &font);
    void findSizes(const QFont &font);
    void insertCharacter(const QString &character);
    void updateClipboard();

private:
    CharacterWidget *characterWidget;
    QClipboard *clipboard;
    QComboBox *styleCombo;
    QComboBox *sizeCombo;
    QFontComboBox *fontCombo;
    QLineEdit *lineEdit;
    QScrollArea *scrollArea;
    QCheckBox *fontMerging;
};

#endif

/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LANGUAGECHOOSER_H
#define LANGUAGECHOOSER_H

#include <QDialog>
#include <QMap>
#include <QStringList>

class QAbstractButton;
class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class MainWindow;

class LanguageChooser : public QDialog
{
    Q_OBJECT

public:
    LanguageChooser(QWidget *parent = 0);

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void checkBoxToggled();
    void showAll();
    void hideAll();

private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);
    QColor colorForLanguage(const QString &language);

    QGroupBox *groupBox;
    QDialogButtonBox *buttonBox;
    QAbstractButton *showAllButton;
    QAbstractButton *hideAllButton;
    QMap<QCheckBox *, QString> qmFileForCheckBoxMap;
    QMap<QCheckBox *, MainWindow *> mainWindowForCheckBoxMap;
};

#endif

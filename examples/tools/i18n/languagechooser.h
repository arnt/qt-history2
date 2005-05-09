/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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

class QCheckBox;
class QGroupBox;
class QPushButton;
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
    QPushButton *showAllButton;
    QPushButton *hideAllButton;
    QPushButton *closeButton;
    QMap<QCheckBox *, QString> qmFileForCheckBoxMap;
    QMap<QCheckBox *, MainWindow *> mainWindowForCheckBoxMap;
};

#endif

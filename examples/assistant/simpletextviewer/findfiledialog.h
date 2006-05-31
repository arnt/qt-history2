/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FINDFILEDIALOG_H
#define FINDFILEDIALOG_H

#include <QDialog>
#include <QAssistantClient>

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QTextEdit;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

class FindFileDialog : public QDialog
{
    Q_OBJECT

public:
    FindFileDialog(QTextEdit *editor, QAssistantClient *assistant,
                   QWidget *parent = 0);

private slots:
    void browse();
    void help();
    void openFile(QTreeWidgetItem *item = 0);
    void update();

private:
    void findFiles();
    void showFiles(const QStringList &files);

    void createButtons();
    void createComboBoxes();
    void createFilesTree();
    void createLabels();
    void createLayout();

    QAssistantClient *currentAssistantClient;
    QTextEdit *currentEditor;
    QTreeWidget *foundFilesTree;

    QComboBox *directoryComboBox;
    QComboBox *fileNameComboBox;

    QLabel *directoryLabel;
    QLabel *fileNameLabel;

    QDialogButtonBox *buttonBox;

    QToolButton *browseButton;
};

#endif

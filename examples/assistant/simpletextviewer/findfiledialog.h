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

#ifndef FINDFILEDIALOG_H
#define FINDFILEDIALOG_H

#include <QAssistantClient>
#include <QDialog>

QT_DECLARE_CLASS(QComboBox)
QT_DECLARE_CLASS(QDialogButtonBox)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QTextEdit)
QT_DECLARE_CLASS(QToolButton)
QT_DECLARE_CLASS(QTreeWidget)
QT_DECLARE_CLASS(QTreeWidgetItem)

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

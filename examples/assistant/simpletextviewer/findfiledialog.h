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
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QTextEdit;
class QToolButton;

class FindFileDialog : public QDialog
{
    Q_OBJECT

public:
    FindFileDialog(QTextEdit *editor, QAssistantClient *assistant,
                   QWidget *parent = 0);

private slots:
    void browse();
    void help();
    void openFile(int row = -1, int column = -1);
    void update();
    void select(QTableWidgetItem *current,
                QTableWidgetItem *previous = 0);

private:
    void findFiles();
    void showFiles(const QStringList &files);

    void createButtons();
    void createComboBoxes();
    void createFilesTable();
    void createLabels();
    void createLayout();

    QAssistantClient *currentAssistantClient;
    QTextEdit *currentEditor;
    QTableWidget *filesFoundTable;

    QComboBox *directoryComboBox;
    QComboBox *fileNameComboBox;

    QLabel *directoryLabel;
    QLabel *fileNameLabel;

    QPushButton *cancelButton;
    QPushButton *helpButton;
    QPushButton *openButton;

    QToolButton *browseButton;
};

#endif

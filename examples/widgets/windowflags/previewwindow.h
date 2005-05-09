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

#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QWidget>

class QPushButton;
class QTextEdit;

class PreviewWindow : public QWidget
{
    Q_OBJECT

public:
    PreviewWindow(QWidget *parent = 0);

    void setWindowFlags(Qt::WindowFlags flags);

private:
    QTextEdit *textEdit;
    QPushButton *closeButton;
};

#endif

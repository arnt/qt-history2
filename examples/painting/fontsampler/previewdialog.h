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

#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include <QPrinter>
#include <QList>
#include <QStringList>
#include "ui_previewdialogbase.h"
#include "previewlabel.h"

QT_DECLARE_CLASS(QTreeWidgetItem)

typedef QList<QTreeWidgetItem *> StyleItems;

Q_DECLARE_METATYPE(StyleItems);

class PreviewDialog : public QDialog, private Ui::PreviewDialogBase
{
    Q_OBJECT

public:
    PreviewDialog(QPrinter &printer, QWidget *parent);
    enum { SmallPreviewLength = 200, LargePreviewLength = 400 };

    bool isSelected(int index);

signals:
    void pageRequested(int index, QPainter &painter, QPrinter &printer);

protected:
    void resizeEvent(QResizeEvent *);

public slots:
    void accept();
    void addPage();
    void on_pageList_currentItemChanged();
    void on_paperSizeCombo_activated(int index);
    void on_paperOrientationCombo_activated(int index);
    void reject();
    void setNumberOfPages(int count);

private:
    void paintItem(QTreeWidgetItem *item, int index);
    void paintPreview(QPixmap &pixmap, int index);
    void setupComboBoxes();

    PreviewLabel *previewLabel;
    QPrinter &printer;
    bool canceled;
    int currentPage;
    int pageCount;
};

#endif

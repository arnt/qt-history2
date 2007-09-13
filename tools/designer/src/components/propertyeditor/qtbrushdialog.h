/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTBRUSHDIALOG_H
#define QTBRUSHDIALOG_H

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QDesignerBrushManagerInterface;

namespace qdesigner_internal {

class QtBrushDialog : public QDialog
{
    Q_OBJECT
public:
    QtBrushDialog(QWidget *parent = 0);
    ~QtBrushDialog();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QDesignerBrushManagerInterface *manager);

signals:
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
private:
    class QtBrushDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushDialog)
    Q_DISABLE_COPY(QtBrushDialog)
};

}

QT_END_NAMESPACE

#endif

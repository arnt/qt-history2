/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#include "QtGui/qwidget.h"

QT_MODULE(Gui)

class QPushButton;
class QDialogPrivate;

class Q_GUI_EXPORT QDialog : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDialog)
    friend class QPushButton;

    Q_PROPERTY(bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled)
    Q_PROPERTY(bool modal READ isModal WRITE setModal)

public:
    explicit QDialog(QWidget *parent = 0, Qt::WFlags f = 0);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QDialog(QWidget *parent, const char *name, bool modal = false,
                                  Qt::WFlags f = 0);
#endif
    ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int result() const;

    void setVisible(bool visible);

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setExtension(QWidget* extension);
    QWidget* extension() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setSizeGripEnabled(bool);
    bool isSizeGripEnabled() const;

    void setModal(bool modal);
#ifdef Q_OS_TEMP
    bool event(QEvent *);
#endif

    void setResult(int r);

public slots:
    int exec();
    virtual void done(int);
    virtual void accept();
    virtual void reject();

    void showExtension(bool);

protected:
    QDialog(QDialogPrivate &, QWidget *parent, Qt::WFlags f = 0);

    void keyPressEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    bool eventFilter(QObject *, QEvent *);
    void adjustPosition(QWidget*);

private:
    Q_DISABLE_COPY(QDialog)
};


#endif // QDIALOG_H

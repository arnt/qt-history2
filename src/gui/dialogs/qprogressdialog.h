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

#ifndef QPROGRESSDIALOG_H
#define QPROGRESSDIALOG_H

#include "QtGui/qdialog.h"

#ifndef QT_NO_PROGRESSDIALOG

class QPushButton;
class QLabel;
class QProgressBar;
class QTimer;
class QProgressDialogPrivate;

class Q_GUI_EXPORT QProgressDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QProgressDialog)
    Q_PROPERTY(bool wasCanceled READ wasCanceled)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(bool autoReset READ autoReset WRITE setAutoReset)
    Q_PROPERTY(bool autoClose READ autoClose WRITE setAutoClose)
    Q_PROPERTY(int minimumDuration READ minimumDuration WRITE setMinimumDuration)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)

public:
    explicit QProgressDialog(QWidget *parent = 0, Qt::WFlags f = 0);
    QProgressDialog(const QString &labelText, const QString &cancelButtonText,
                    int minimum, int maximum,
                    QWidget *parent = 0, Qt::WFlags f = 0);
    ~QProgressDialog();

    void setLabel(QLabel *label);
    void setCancelButton(QPushButton *button);
    void setBar(QProgressBar *bar);

    bool wasCanceled() const;

    int minimum() const;
    int maximum() const;

    void setRange(int minimum, int maximum);
    int value() const;

    QSize sizeHint() const;

    QString labelText() const;
    int minimumDuration() const;

    void setAutoReset(bool b);
    bool autoReset() const;
    void setAutoClose(bool b);
    bool autoClose() const;

public slots:
    void cancel();
    void reset();
    void setMaximum(int maximum);
    void setMinimum(int minimum);
    void setValue(int progress);
    void setLabelText(const QString &);
    void setCancelButtonText(const QString &);
    void setMinimumDuration(int ms);

signals:
    void canceled();

protected:
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *);
    void showEvent(QShowEvent *e);

protected slots:
    void forceShow();

private:
    Q_DISABLE_COPY(QProgressDialog)
};

#endif // QT_NO_PROGRESSDIALOG

#endif // QPROGRESSDIALOG_H

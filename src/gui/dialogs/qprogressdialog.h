/****************************************************************************
**
** Definition of QProgressDialog class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPROGRESSDIALOG_H
#define QPROGRESSDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#include "qlabel.h"       // ### remove or keep for users' convenience?
#include "qprogressbar.h" // ### remove or keep for users' convenience?
#endif // QT_H

#ifndef QT_NO_PROGRESSDIALOG

class QButton;
class QTimer;
class QProgressDialogData;

class Q_GUI_EXPORT QProgressDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool wasCancelled READ wasCancelled DESIGNABLE false STORED false) // ### remove in 4.0
    Q_PROPERTY(bool wasCanceled READ wasCanceled)
    Q_PROPERTY(int totalSteps READ totalSteps WRITE setTotalSteps)
    Q_PROPERTY(int progress READ progress WRITE setProgress)
    Q_PROPERTY(bool autoReset READ autoReset WRITE setAutoReset)
    Q_PROPERTY(bool autoClose READ autoClose WRITE setAutoClose)
    Q_PROPERTY(int minimumDuration READ minimumDuration WRITE setMinimumDuration)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)

public:
    QProgressDialog(QWidget* parent=0, const char* name=0, bool modal=false,
                     WFlags f=0);
    QProgressDialog(const QString& labelText, const QString &cancelButtonText,
                     int totalSteps, QWidget* parent=0, const char* name=0,
                     bool modal=false, WFlags f=0);
    ~QProgressDialog();

    void        setLabel(QLabel *);
    void        setCancelButton(QButton *);
    void        setBar(QProgressBar *);

    // ### Qt 4.0: remove wasCancelled() in 4.0
    bool        wasCanceled() const;
    inline bool        wasCancelled() const { return wasCanceled(); }

    int                totalSteps() const;
    int                progress()   const;

    QSize        sizeHint() const;

    QString     labelText() const;

    void setAutoReset(bool b);
    bool autoReset() const;
    void setAutoClose(bool b);
    bool autoClose() const;

public slots:
    void        cancel();
    void        reset();
    void        setTotalSteps(int totalSteps);
    void        setProgress(int progress);
    void        setProgress(int progress, int totalSteps);
    void        setLabelText(const QString &);
    void        setCancelButtonText(const QString &);

    void        setMinimumDuration(int ms);
public:
    int                minimumDuration() const;

signals:
    // ### remove cancelled() in 4.0
    void        cancelled();
    void        canceled();

protected:
    void        resizeEvent(QResizeEvent *);
    void        closeEvent(QCloseEvent *);
    void        changeEvent(QEvent *);
    void        showEvent(QShowEvent *e);

protected slots:
    void        forceShow();

private:
    void           init(QWidget *creator, const QString& lbl, const QString &canc,
                         int totstps);
    void           layout();
    QLabel          *label()  const;
    QProgressBar  *bar()    const;
    QProgressDialogData *d;
    QTimer          *forceTimer;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QProgressDialog(const QProgressDialog &);
    QProgressDialog &operator=(const QProgressDialog &);
#endif
};

#endif // QT_NO_PROGRESSDIALOG

#endif // QPROGRESSDIALOG_H

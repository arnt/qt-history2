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

#ifndef Q3PROGRESSBAR_H
#define Q3PROGRESSBAR_H

#include "QtGui/qframe.h"

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_PROGRESSBAR

class Q3ProgressBarPrivate;

class Q_COMPAT_EXPORT Q3ProgressBar : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int totalSteps READ totalSteps WRITE setTotalSteps)
    Q_PROPERTY(int progress READ progress WRITE setProgress)
    Q_PROPERTY(QString progressString READ progressString)
    Q_PROPERTY(bool centerIndicator READ centerIndicator WRITE setCenterIndicator)
    Q_PROPERTY(bool percentageVisible READ percentageVisible WRITE setPercentageVisible)

public:
    Q3ProgressBar(QWidget *parent, const char *name, Qt::WFlags f=0);
    Q3ProgressBar(int totalSteps, QWidget *parent, const char *name,
                  Qt::WFlags f=0);
    Q3ProgressBar(QWidget *parent = 0, Qt::WFlags f = 0);
    Q3ProgressBar(int totalSteps, QWidget *parent = 0, Qt::WFlags f=0);

    int totalSteps() const;
    int progress() const;
    const QString &progressString() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setCenterIndicator(bool on);
    bool centerIndicator() const;

    bool percentageVisible() const;
    void setPercentageVisible(bool);

    void setVisible(bool visible);

public slots:
    void reset();
    virtual void setTotalSteps(int totalSteps);
    virtual void setProgress(int progress);
    void setProgress(int progress, int totalSteps);

protected:
    void paintEvent(QPaintEvent *);
    virtual bool setIndicator(QString &progress_str, int progress, int totalSteps);
    void changeEvent(QEvent *);

private:
    Q_DISABLE_COPY(Q3ProgressBar)

    int total_steps;
    int progress_val;
    int percentage;
    QString progress_str;
    bool center_indicator : 1;
    bool percentage_visible : 1;
    Q3ProgressBarPrivate *d;
    void initFrame();
};


inline int Q3ProgressBar::totalSteps() const
{
    return total_steps;
}

inline int Q3ProgressBar::progress() const
{
    return progress_val;
}

inline const QString &Q3ProgressBar::progressString() const
{
    return progress_str;
}

inline bool Q3ProgressBar::centerIndicator() const
{
    return center_indicator;
}

inline bool Q3ProgressBar::percentageVisible() const
{
    return percentage_visible;
}

#endif // QT_NO_PROGRESSBAR

#endif // Q3PROGRESSBAR_H

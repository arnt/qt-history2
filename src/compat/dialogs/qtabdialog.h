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

#ifndef QTABDIALOG_H
#define QTABDIALOG_H

#include "qdialog.h"

#ifndef QT_NO_TABDIALOG

class QIcon;
class QTabBar;
class QTabDialogPrivate;

class Q_COMPAT_EXPORT QTabDialog : public QDialog
{
    Q_OBJECT
public:
    QTabDialog(QWidget *parent=0, const char *name=0, bool modal=false,
                Qt::WFlags f=0);
    ~QTabDialog();

    void show();
    void setFont(const QFont &font);

    void addTab(QWidget *, const QString &);
    void addTab(QWidget *child, const QIcon &icon, const QString &label);

    void insertTab(QWidget *, const QString &, int index = -1);
    void insertTab(QWidget *child, const QIcon &icon, const QString &label, int index = -1);

    void changeTab(QWidget *, const QString &);
    void changeTab(QWidget *child, const QIcon &icon, const QString &label);

    bool isTabEnabled( QWidget *) const;
    void setTabEnabled(QWidget *, bool);
    bool isTabEnabled(const char *) const;
    void setTabEnabled(const char *, bool);

    void showPage(QWidget *);
    void removePage(QWidget *);
    QString tabLabel(QWidget *) const;

    QWidget * currentPage() const;

    void setDefaultButton(const QString &text);
    void setDefaultButton();
    bool hasDefaultButton() const;

    void setHelpButton(const QString &text);
    void setHelpButton();
    bool hasHelpButton() const;

    void setCancelButton(const QString &text);
    void setCancelButton();
    bool hasCancelButton() const;

    void setApplyButton(const QString &text);
    void setApplyButton();
    bool hasApplyButton() const;

    void setOkButton(const QString &text);
    void setOkButton();
    inline void setOKButton(const QString &text = QString())
        { if (text.isEmpty()) setOkButton(QLatin1String("OK")); else setOkButton(text); }
    bool hasOkButton() const;

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent *);
    void setTabBar(QTabBar*);
    QTabBar* tabBar() const;

signals:
    void aboutToShow();

    void applyButtonPressed();
    void cancelButtonPressed();
    void defaultButtonPressed();
    void helpButtonPressed();

    void currentChanged(QWidget *);
    void selected(const QString &);

private slots:
    void showTab(int i);

private:
    Q_DISABLE_COPY(QTabDialog)

    void setSizes();
    void setUpLayout();

    QTabDialogPrivate *d;
};

#endif // QT_NO_TABDIALOG

#endif // QTABDIALOG_H

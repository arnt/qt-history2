/****************************************************************************
**
** Definition of the QWizard class.
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

#ifndef QWIZARD_H
#define QWIZARD_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_WIZARD

class QHBoxLayout;
class QWizardPrivate;

class Q_COMPAT_EXPORT QWizard : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QFont titleFont READ titleFont WRITE setTitleFont)

public:
    QWizard(QWidget* parent=0, const char* name=0, bool modal=false,
             Qt::WFlags f=0);
    ~QWizard();

    void show();

    virtual void addPage(QWidget *, const QString &);
    virtual void insertPage(QWidget*, const QString&, int);
    virtual void removePage(QWidget *);

    QString title(QWidget *) const;
    void setTitle(QWidget *, const QString &);
    QFont titleFont() const;
    void setTitleFont(const QFont &);

    virtual void showPage(QWidget *);

    QWidget * currentPage() const;

    QWidget* page(int) const;
    int pageCount() const;
    int indexOf(QWidget*) const;

    virtual bool appropriate(QWidget *) const;
    virtual void setAppropriate(QWidget *, bool);

    QPushButton * backButton() const;
    QPushButton * nextButton() const;
    QPushButton * finishButton() const;
    QPushButton * cancelButton() const;
    QPushButton * helpButton() const;

    bool eventFilter(QObject *, QEvent *);

public slots:
    virtual void setBackEnabled(QWidget *, bool);
    virtual void setNextEnabled(QWidget *, bool);
    virtual void setFinishEnabled(QWidget *, bool);

    virtual void setHelpEnabled(QWidget *, bool);

    // obsolete
    virtual void setFinish( QWidget *, bool) {}

protected slots:
    virtual void back();
    virtual void next();
    virtual void help();

signals:
    void helpClicked();
    void selected(const QString&);

protected:
    void changeEvent(QEvent *event);
    virtual void layOutButtonRow(QHBoxLayout *);
    virtual void layOutTitleRow(QHBoxLayout *, const QString &);

private:
    void setBackEnabled(bool);
    void setNextEnabled(bool);

    void setHelpEnabled(bool);

    void setNextPage(QWidget *);

    void updateButtons();

    void layOut();

    QWizardPrivate *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWizard(const QWizard &);
    QWizard& operator=(const QWizard &);
#endif
};

#endif // QT_NO_WIZARD
#endif // QWIZARD_H

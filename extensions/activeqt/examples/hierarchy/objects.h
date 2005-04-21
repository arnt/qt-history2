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

#ifndef OBJECTS_H
#define OBJECTS_H

#include <qwidget.h>

class QVBoxLayout;
class QSubWidget;

class QParentWidget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{d574a747-8016-46db-a07c-b2b4854ee75c}");
    Q_CLASSINFO("InterfaceID", "{4a30719d-d9c2-4659-9d16-67378209f822}");
public:
    QParentWidget(QWidget *parent = 0);

    QSize sizeHint() const;

public slots:
    void createSubWidget( const QString &name );

    QSubWidget *subWidget( const QString &name );

private:
    QVBoxLayout *vbox;
};

class QSubWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )

    Q_CLASSINFO("ClassID", "{850652f4-8f71-4f69-b745-bce241ccdc30}");
    Q_CLASSINFO("InterfaceID", "{2d76cc2f-3488-417a-83d6-debff88b3c3f}");
    Q_CLASSINFO("ToSuperClass", "QSubWidget");
    
public:
    QSubWidget(QWidget *parent = 0, const QString &name = QString());

    void setLabel( const QString &text );
    QString label() const;

    QSize sizeHint() const;

protected:
    void paintEvent( QPaintEvent *e );

private:
    QString lbl;
};

#endif // OBJECTS_H

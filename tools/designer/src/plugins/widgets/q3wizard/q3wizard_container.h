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

#ifndef Q3WIZARD_CONTAINER_H
#define Q3WIZARD_CONTAINER_H

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionFactory>

QT_BEGIN_NAMESPACE

class Q3Wizard;

class Q3WizardContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit Q3WizardContainer(Q3Wizard *wizard, QObject *parent = 0);

    virtual int count() const;
    virtual QWidget *widget(int index) const;
    virtual int currentIndex() const;
    virtual void setCurrentIndex(int index);
    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    Q3Wizard *m_wizard;
};

class Q3WizardContainerFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    explicit Q3WizardContainerFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

QT_END_NAMESPACE

#endif // Q3WIZARD_CONTAINER_H

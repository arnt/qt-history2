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

#ifndef QDESIGNER_INTEGRATION_H
#define QDESIGNER_INTEGRATION_H

#include <QtCore/QObject>

class AbstractFormEditor;
class AbstractFormWindow;
class AbstractFormWindowManager;

class QVariant;
class QWidget;

class QDesignerIntegration: public QObject
{
    Q_OBJECT
public:
    QDesignerIntegration(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~QDesignerIntegration();

    inline AbstractFormEditor *core() const;

signals:
    void propertyChanged(AbstractFormWindow *formWindow, const QString &name, const QVariant &value);

public slots:
    virtual void updateProperty(const QString &name, const QVariant &value);
    virtual void updateActiveFormWindow(AbstractFormWindow *formWindow);
    virtual void setupFormWindow(AbstractFormWindow *formWindow);
    virtual void updateSelection();
    virtual void updateGeometry();
    virtual void activateWidget(QWidget *widget);

private:
    void initialize();

private:
    AbstractFormEditor *m_core;
    AbstractFormWindowManager *m_formWindowManager;
};

inline AbstractFormEditor *QDesignerIntegration::core() const
{ return m_core; }


#endif // QDESIGNER_INTEGRATION_H

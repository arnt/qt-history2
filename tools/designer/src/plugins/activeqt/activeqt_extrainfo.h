/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ACTIVEQT_EXTRAINFO_H
#define ACTIVEQT_EXTRAINFO_H

#include <QtDesigner/QDesignerExtraInfoExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionFactory>

#include <QtCore/QPointer>
#include <QWidget>

#include <QPixmap>

class QAxWidget;

class QActiveXPluginObject : public QWidget
{
public:
    QActiveXPluginObject(QWidget *parent);
    ~QActiveXPluginObject();

    const QMetaObject *metaObject() const;
    int qt_metacall(QMetaObject::Call, int, void **);
    static const QMetaObject QActiveXPluginObject::staticMetaObject;

    bool setControl(const QString &clsid);
    QSize sizeHint() const;

    bool loaded() { return (m_axobject != 0); }
protected:
    void paintEvent (QPaintEvent *event);

    void resizeControlPixmap();
    void cleanup();
private:
    QAxWidget *m_axobject;
    QPixmap m_axImage;
    QMap<int, QVariant *> m_propValues;
};

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QActiveXPluginObject *qobject_cast_helper<QActiveXPluginObject*>(QObject *o, QActiveXPluginObject *)
#else
template <> inline QActiveXPluginObject *qobject_cast<QActiveXPluginObject*>(QObject *o)
#endif
{
    void *result = 0;

    if (o && strcmp(o->metaObject()->className(), "QAxWidget") == 0)
        result = static_cast<QActiveXPluginObject*>(o);

    return (QActiveXPluginObject*)(result);
}

class QAxWidgetExtraInfo: public QObject, public QDesignerExtraInfoExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerExtraInfoExtension)
public:
    QAxWidgetExtraInfo(QActiveXPluginObject *widget, QDesignerFormEditorInterface *core, QObject *parent);

    virtual QWidget *widget() const;
    virtual QDesignerFormEditorInterface *core() const;

    virtual bool saveUiExtraInfo(DomUI *ui);
    virtual bool loadUiExtraInfo(DomUI *ui);

    virtual bool saveWidgetExtraInfo(DomWidget *ui_widget);
    virtual bool loadWidgetExtraInfo(DomWidget *ui_widget);

private:
    QPointer<QActiveXPluginObject> m_widget;
    QPointer<QDesignerFormEditorInterface> m_core;
};

class QAxWidgetExtraInfoFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    QAxWidgetExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

private:
    QDesignerFormEditorInterface *m_core;
};

#endif // ACTIVEQT_EXTRAINFO_H

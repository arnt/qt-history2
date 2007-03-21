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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_ABTRACT_LANGUAGE_H
#define QDESIGNER_ABTRACT_LANGUAGE_H

#include <QtDesigner/extension.h>

QT_BEGIN_HEADER

class QDialog;
class QWidget;
class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QDesignerResourceBrowserInterface;

class QDesignerLanguageExtension
{
public:
    virtual ~QDesignerLanguageExtension() {}

    virtual QDialog *createFormWindowSettingsDialog(QDesignerFormWindowInterface *formWindow, QWidget *parentWidget) = 0;
    virtual QDesignerResourceBrowserInterface *createResourceBrowser(QWidget *parentWidget) = 0;

    virtual QDialog *createPromotionDialog(QDesignerFormEditorInterface *formEditor, QWidget *parentWidget = 0) = 0;

    virtual QDialog *createPromotionDialog(QDesignerFormEditorInterface *formEditor,
                                           const QString &promotableWidgetClassName,
                                           QString *promoteToClassName,
                                           QWidget *parentWidget = 0) = 0;

    virtual bool isLanguageResource(const QString &path) const = 0;

    virtual QString classNameOf(QObject *object) const = 0;

    virtual bool signalMatchesSlot(const QString &signal, const QString &slot) const = 0;

    virtual QString enumerator(const QString &neutralName) const = 0;
    virtual QString neutralEnumerator(const QString &enumName) const = 0;

    virtual QString widgetBoxContents() const = 0;

    virtual QString uiExtension() const = 0;
};

Q_DECLARE_EXTENSION_INTERFACE(QDesignerLanguageExtension, "com.trolltech.Qt.Designer.Language.3")

QT_END_HEADER

#endif // QDESIGNER_ABTRACT_LANGUAGE_H

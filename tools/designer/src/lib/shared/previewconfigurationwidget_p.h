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

#ifndef PREVIEWCONFIGURATIONWIDGET_H
#define PREVIEWCONFIGURATIONWIDGET_H

#include "shared_global_p.h"

#include <QtGui/QGroupBox>

QT_BEGIN_NAMESPACE

class QSettings;

namespace qdesigner_internal {

struct PreviewConfiguration;

// ----------- PreviewConfigurationWidgetState: Additional state that goes
//             to QSettings besides the actual PreviewConfiguration (enabled flag and list of user deviceSkins).

struct QDESIGNER_SHARED_EXPORT PreviewConfigurationWidgetState {
    PreviewConfigurationWidgetState();
    PreviewConfigurationWidgetState(const QStringList &userDeviceSkins, bool enabled);

    // Returns the PreviewConfiguration according to the enabled flag.
    PreviewConfiguration previewConfiguration(const PreviewConfiguration &serializedConfiguration) const;

    void clear();
    void toSettings(const QString &prefix, QSettings &settings) const;
    void fromSettings(const QString &prefix, const QSettings &settings);

    bool enabled;
    QStringList userDeviceSkins;
};

// ----------- PreviewConfigurationWidget: Widget to edit the preview configuration.

class QDESIGNER_SHARED_EXPORT PreviewConfigurationWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit PreviewConfigurationWidget(QWidget *parent = 0);
    virtual ~PreviewConfigurationWidget();

    PreviewConfigurationWidgetState previewConfigurationWidgetState() const;
    void setPreviewConfigurationWidgetState(const PreviewConfigurationWidgetState &pc);

    // Note: These accessors are for serialization only; to determine
    // the actual PreviewConfiguration, the enabled flag of PreviewConfigurationWidgetState has to be observed.
    PreviewConfiguration previewConfiguration() const;
    void setPreviewConfiguration(const PreviewConfiguration &pc);

private slots:
    void slotEditAppStyleSheet();
    void slotDeleteSkinEntry();
    void slotSkinChanged(int);

private:
    class PreviewConfigurationWidgetPrivate;
    PreviewConfigurationWidgetPrivate *m_impl;

    PreviewConfigurationWidget(const PreviewConfigurationWidget &other);
    PreviewConfigurationWidget &operator =(const PreviewConfigurationWidget &other);
};
}

QT_END_NAMESPACE

#endif // PREVIEWCONFIGURATIONWIDGET_H

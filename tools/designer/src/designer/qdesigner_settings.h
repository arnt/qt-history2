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

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include <QtCore/QRect>
#include <QtCore/QSettings>

class QHeaderView;
class QDesignerSettings : public QSettings
{
public:
    QDesignerSettings();
    virtual ~QDesignerSettings();

    QStringList formTemplatePaths() const;
    void setFormTemplatePaths(const QStringList &paths);

    void setGeometryFor(QWidget *w, const QRect &fallBack = QRect()) const;
    void saveGeometryFor(const QWidget *w);

    void setHeaderSizesFor(QHeaderView *hv) const;
    void saveHeaderSizesFor(const QHeaderView *hv);
    QStringList recentFilesList() const;
    void setRecentFilesList(const QStringList &list);

    void setShowNewFormOnStartup(bool showIt);
    bool showNewFormOnStartup() const;

    void setUIMode(int mode);
    int uiMode() const;

    void setUseBigIcons(bool useBig);
    bool useBigIcons() const;

private:
    QStringList defaultFormTemplatePaths() const;

    void setGeometryHelper(QWidget *w, const QString &key, const QRect &fallBack) const;
    void saveGeometryHelper(const QWidget *w, const QString &key);

    void setHeaderSizesForHelper(QHeaderView *hv, const QString &key,
                           const QList<QVariant> &hints) const;
    void saveHeaderSizesForHelper(const QHeaderView *hv, const QString &key);
private:
    QString m_designerPath;
};

#endif // QDESIGNER_SETTINGS_H

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
// of the Qt tools.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef FONTPANEL_H
#define FONTPANEL_H

#include <QtGui/QGroupBox>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

class QComboBox;
class QFontComboBox;
class QTimer;
class QLineEdit;

class FontPanel: public QGroupBox
{
    Q_OBJECT
public:
    FontPanel(QWidget *parentWidget = 0);

    QFont selectedFont() const;
    void setSelectedFont(const QFont &);
        
    QFontDatabase::WritingSystem writingSystem() const;
    void setWritingSystem(QFontDatabase::WritingSystem ws);

private slots:
    void slotWritingSystemChanged(int);
    void slotFamilyChanged(int);
    void slotStyleChanged(int);
    void slotPointSizeChanged(int);
    void slotUpdatePreviewFont();

private:
    QString family() const;
    QString styleString() const;
    int pointSize() const;

    void updateWritingSystem(QFontDatabase::WritingSystem ws);
    void updateFamily(const QString &family);
    void updatePointSizes(const QString &family, const QString &style);
    void delayedPreviewFontUpdate();
    
    QFontDatabase m_fontDatabase;
    QLineEdit *m_previewLineEdit;
    QComboBox *m_writingSystemComboBox;
    QFontComboBox* m_familyComboBox;
    QComboBox *m_styleComboBox;
    QComboBox *m_pointSizeComboBox;
    QTimer *m_previewFontUpdateTimer;
};

#endif // FONTPANEL_H

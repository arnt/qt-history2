#ifndef FINDICONDIALOG_H
#define FINDICONDIALOG_H

#include "propertyeditor_global.h"
#include "ui_findicondialog.h"

class AbstractFormWindow;

class QT_PROPERTYEDITOR_EXPORT FindIconDialog : public QDialog,
                                                public Ui::FindIconDialog
{
    Q_OBJECT
    
public:
    FindIconDialog(AbstractFormWindow *form, QWidget *parent);
    void setPaths(const QString &qrcPath, const QString &filePath);
    QString qrcPath() const;
    QString filePath() const;

private slots:
    void updateBoxes();
    void imageFileSelected(QListWidgetItem*);
    void browseFileDir();
    void setActiveBox();
    
private:
    enum InputBox { FileBox, ResourceBox };
    
    void activateBox(InputBox box);
    InputBox activeBox() const;

    QString m_icon_file_name;
    AbstractFormWindow *m_form;
};

#endif // FINDICONDIALOG_H

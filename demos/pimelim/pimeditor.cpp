#include "pimeditor.h"
#include "pimmodel.h"
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qicon.h>
#include <qlayout.h>

PimEditor::PimEditor(QWidget *parent)
    : QWidget(parent),
      m(0), entry(-1),
      photoButton(0),
      firstNameEdit(0),
      lastNameEdit(0),
      middleNameEdit(0),
      companyEdit(0),
      departmentEdit(0),
      jobTitleEdit(0),
      acceptButton(0),
      cancelButton(0)
{
    QGridLayout *grid = new QGridLayout(this);

    photoButton = new QPushButton(this);
    grid->addWidget(photoButton, 0, 0, 1, 2, Qt::AlignCenter);
    connect(photoButton, SIGNAL(clicked()), this, SLOT(photo()));

    grid->addWidget(new QLabel("First Name", this), 1, 0);
    firstNameEdit = new QLineEdit(this);
    grid->addWidget(firstNameEdit, 1, 1, Qt::AlignHCenter);

    grid->addWidget(new QLabel("Last Name", this), 2, 0);
    lastNameEdit = new QLineEdit(this);
    grid->addWidget(lastNameEdit, 2, 1, Qt::AlignHCenter);

    grid->addWidget(new QLabel("Middle Name", this), 3, 0);
    middleNameEdit = new QLineEdit(this);
    grid->addWidget(middleNameEdit, 3, 1, Qt::AlignHCenter);

    grid->addWidget(new QLabel("Company", this), 4, 0);
    companyEdit = new QLineEdit(this);
    grid->addWidget(companyEdit, 4, 1, Qt::AlignHCenter);

    grid->addWidget(new QLabel("Department", this), 5, 0);
    departmentEdit = new QLineEdit(this);
    grid->addWidget(departmentEdit, 5, 1, Qt::AlignHCenter);

    grid->addWidget(new QLabel("Job Title", this), 6, 0);
    jobTitleEdit = new QLineEdit(this);
    grid->addWidget(jobTitleEdit, 6, 1, Qt::AlignHCenter);

    acceptButton = new QPushButton("&Accept", this);
    grid->addWidget(acceptButton, 7, 0, 1, 2);
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));

    cancelButton = new QPushButton("&Cancel", this);
    grid->addWidget(cancelButton, 8, 0, 1, 2);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

    grid->setResizeMode(QLayout::Fixed);
}

PimEditor::~PimEditor()
{

}

void PimEditor::setModel(PimModel *model)
{
    m = model;
}

PimModel *PimEditor::model() const
{
    return m;
}

void PimEditor::create()
{
    if (m) {
        entry = m->rowCount();
        photoButton->setIcon(QIcon());
        firstNameEdit->clear();
        lastNameEdit->clear();
        middleNameEdit->clear();
        companyEdit->clear();
        departmentEdit->clear();
        jobTitleEdit->clear();
    }
}

void PimEditor::edit(const QModelIndex &index)
{
    if (index.isValid()) {
        entry = index.row();
        const PimEntry &pe = m->entry(entry);
        photoButton->setIcon(pe.photo);
        firstNameEdit->setText(pe.firstName);
        lastNameEdit->setText(pe.lastName);
        middleNameEdit->setText(pe.middleName);
        companyEdit->setText(pe.company);
        departmentEdit->setText(pe.department);
        jobTitleEdit->setText(pe.jobTitle);
    }
}

void PimEditor::accept()
{
    PimEntry pe;
    pe.photo = photoButton->icon().pixmap(QSize(64, 64));
    pe.firstName = firstNameEdit->text();
    pe.lastName = lastNameEdit->text();
    pe.middleName = middleNameEdit->text();
    pe.company = companyEdit->text();
    pe.department = departmentEdit->text();
    pe.jobTitle = jobTitleEdit->text();
    if (entry >= m->rowCount())
        m->appendEntry(pe);
    else
        m->setEntry(entry, pe);
    entry = -1;
    emit done();
}

void PimEditor::cancel()
{
    entry = -1;
    emit done();
}

void PimEditor::photo()
{
    QString lastName = lastNameEdit->text();
    QString photoName = QFileDialog::getOpenFileName(0, QObject::tr("Open Image"),
                                                lastName + ".png", "*.png;;*.jpg;;*.*");
    if (!photoName.isEmpty()) {
        QPixmap photo(photoName);
        photoButton->setIcon(photo);
    }
}


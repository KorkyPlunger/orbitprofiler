#include "licensedialog.h"
#include "ui_licensedialog.h"
#include <QPlainTextEdit>

using namespace std;

LicenseDialog::LicenseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LicenseDialog)
{
    ui->setupUi(this);
}

LicenseDialog::~LicenseDialog()
{
    delete ui;
}

wstring LicenseDialog::GetLicense()
{
    return ui->LicenseTextEdit->document()->toPlainText().toStdWString();
}
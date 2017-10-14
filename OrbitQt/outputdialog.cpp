#include "outputdialog.h"
#include "ui_outputdialog.h"

using namespace std;

//-----------------------------------------------------------------------------
OutputDialog::OutputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OutputDialog)
{
    ui->setupUi(this);
}

//-----------------------------------------------------------------------------
OutputDialog::~OutputDialog()
{
    delete ui;
}

//-----------------------------------------------------------------------------
void OutputDialog::Reset()
{
    ui->OutputTextEdit->setPlainText("");
    ui->staticTextEdit->setPlainText("");
}

//-----------------------------------------------------------------------------
void OutputDialog::SetStatus( const string & a_Status )
{
    ui->staticTextEdit->setPlainText( a_Status.c_str() );
}

//-----------------------------------------------------------------------------
void OutputDialog::AddLog( const wstring & a_Log )
{
    wstring log = ui->OutputTextEdit->toPlainText().toStdWString() + a_Log;
    ui->OutputTextEdit->setPlainText( QString::fromWCharArray(log.c_str()) );
}
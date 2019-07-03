#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->pbReloadSettings->animateClick();
}

Widget::~Widget()
{
    delete ui;
}

QString Widget::getCommandText() const
{
    return ui->leCommand->text().simplified().trimmed();
}

QString Widget::getProjectDirectory() const
{
    return ui->leProjectDir->text();
}

void Widget::reloadSettings()
{
    ui->pteLog->appendPlainText(tr("Settings are reloading"));
    QSettings settings(QSettings::IniFormat, "my-organization-name", qAppName());
    settings.beginGroup("uploader");
    ui->leCommand->setText(settings.value("leCommand").toString());//scp <file name> <user>@<dest>:<path on a server>
    ui->leProjectDir->setText(settings.value("leProjectDir").toString());

    ui->ptePaths->setPlainText(settings.value("ptePaths").toString());
    checkADirectoryAndACommand();
}

void Widget::saveSettings()
{
    QSettings settings(QSettings::IniFormat, "my-organization-name", qAppName());
    settings.beginGroup("uploader");

    settings.setValue("leCommand", getCommandText());
    settings.setValue("leProjectDir", getProjectDirectory());
    settings.setValue("ptePaths", ui->ptePaths->toPlainText());

    ui->pteLog->appendPlainText(tr("Settings were saved"));
}

void Widget::checkADirectoryAndACommand()
{



    QStringList listErrors;
    if(getCommandText().isEmpty())
        listErrors.append(tr("Bad command"));

    if(ui->leProjectDir->text().isEmpty()){
        listErrors.append(tr("Bad project directory"));
    }else{
        QDir dir(ui->leProjectDir->text());
        if(!dir.isReadable())
            listErrors.append(tr("Directory is not readable"));
    }

    if(!listErrors.isEmpty()){
        ui->pteLog->appendPlainText(listErrors.join("\n"));
        ui->pbUploadAll->setEnabled(false);
        ui->pbUploadNew->setEnabled(false);
        return;
    }
    ui->pbUploadAll->setEnabled(true);
    ui->pbUploadNew->setEnabled(true);



}

void Widget::on_pbReloadSettings_clicked()
{
    reloadSettings();
}

void Widget::on_pbSaveSettings_clicked()
{
    saveSettings();
}

void Widget::on_toolButton_clicked()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                          ui->leProjectDir->text(),
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty())
        return;

    ui->leProjectDir->setText(dir);
    checkADirectoryAndACommand();
}

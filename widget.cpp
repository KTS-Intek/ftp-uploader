#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>

#include "filesearcher.h"

//-----------------------------------------------------------------------------------------

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    checkAppArguments();
}

//-----------------------------------------------------------------------------------------

Widget::~Widget()
{
    delete ui;
}

QString Widget::getSettingsFileName() const
{
    return QString("%1.conf").arg(qAppName());
}

//-----------------------------------------------------------------------------------------

QString Widget::getCommandText() const
{
    return ui->leCommand->text().simplified().trimmed();
}

//-----------------------------------------------------------------------------------------

QString Widget::getCommandDirText() const
{
    return ui->leCommandDir->text().simplified().trimmed();

}

//-----------------------------------------------------------------------------------------

QString Widget::getProjectDirectory() const
{
    return ui->leProjectDir->text();
}

//-----------------------------------------------------------------------------------------

QString Widget::getCreateAdirCommand() const
{
    return ui->leCreateAdirectory->text().simplified().trimmed();
}

//-----------------------------------------------------------------------------------------

QString Widget::showHelp() const
{
    QStringList l;
    l.append("-h\tShow this help");
    l.append("-all\tUpload all");
    l.append("-new\tUpload new files");
    return l.join("\n");
}

//-----------------------------------------------------------------------------------------

void Widget::reloadSettings()
{
    append2log(tr("Settings are reloading"));
    QSettings settings(getSettingsFileName(), QSettings::IniFormat);
    settings.beginGroup("uploader");
    ui->leCommand->setText(settings.value("leCommand").toString());//scp <file name> <user>@<dest>:<path on a server>
    ui->leCommandDir->setText(settings.value("leCommandDir").toString());//scp -r <dir name> <user>@<dest>:<path on a server>

    ui->leCreateAdirectory->setText(settings.value("leCreateAdirectory").toString());//matilda1-ssh "mkdir -p /path/2/app/dir"

    ui->leProjectDir->setText(settings.value("leProjectDir").toString());
    ui->pteSubDirs->setPlainText(settings.value("pteSubDirs").toString());
    ui->ptePaths->setPlainText(settings.value("ptePaths").toString());
    checkADirectoryAndACommand();
}

//-----------------------------------------------------------------------------------------

void Widget::saveSettings()
{
    QSettings settings(getSettingsFileName(), QSettings::IniFormat);
    settings.beginGroup("uploader");

    settings.setValue("leCommand", getCommandText());
    settings.setValue("leCommandDir", getCommandDirText());
    settings.setValue("leCreateAdirectory", getCreateAdirCommand());

    settings.setValue("leProjectDir", getProjectDirectory());
    settings.setValue("ptePaths", ui->ptePaths->toPlainText());
    settings.setValue("pteSubDirs", ui->pteSubDirs->toPlainText());

    append2log(tr("Settings were saved"));
}

//-----------------------------------------------------------------------------------------

void Widget::checkADirectoryAndACommand()
{



    QStringList listErrors;
    if(getCommandText().isEmpty())
        listErrors.append(tr("Bad command for files"));

    if(getCommandDirText().isEmpty())
        listErrors.append(tr("Bad command for directories"));


    if(ui->leProjectDir->text().isEmpty()){
        listErrors.append(tr("Bad project directory"));
    }else{
        QDir dir(ui->leProjectDir->text());
        if(!dir.isReadable())
            listErrors.append(tr("Directory is not readable"));
    }

    if(!listErrors.isEmpty()){
        append2log(listErrors.join("\n"));
        ui->pbUploadAll->setEnabled(false);
        ui->pbUploadNew->setEnabled(false);
        return;
    }

    ui->pbUploadAll->setEnabled(!myuploadstate.isUploading);
    ui->pbUploadNew->setEnabled(!myuploadstate.isUploading);



}
//-----------------------------------------------------------------------------------------


void Widget::checkAppArguments()
{
    QStringList args = qApp->arguments();
    if(args.size() < 2){
        ui->pbReloadSettings->animateClick();
        return;
    }

    setVisible(false);


    if(args.contains("-h")){
        qDebug() << showHelp();
       args.clear();
    }

    if(!args.isEmpty())
        reloadSettings();

    if(args.contains("-all")){
        myuploadstate.closeAppOnFinish = true;
        if(try2upload(true)){

        }
        args.clear();
    }

    if(args.contains("-new")){
        myuploadstate.closeAppOnFinish = true;
        if(try2upload(false)){

        }
        args.clear();
    }

    qApp->exit(0);
    return;


}
//-----------------------------------------------------------------------------------------


void Widget::append2log(const QString &ptetext)
{
    qDebug() << ptetext;
    ui->pteLog->appendPlainText(ptetext);

}

//-----------------------------------------------------------------------------------------

void Widget::onUploadingFinished()
{
    append2log(tr("Done!"));
    myuploadstate.isUploading = false;
    if(myuploadstate.closeAppOnFinish){
        qApp->exit(0);
        return;
    }
    checkADirectoryAndACommand();

}

//-----------------------------------------------------------------------------------------

void Widget::on_pbReloadSettings_clicked()
{
    reloadSettings();
}

//-----------------------------------------------------------------------------------------

void Widget::on_pbSaveSettings_clicked()
{
    saveSettings();
}

//-----------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------

void Widget::on_pbUploadAll_clicked()
{
    try2upload(true);
}

//-----------------------------------------------------------------------------------------

void Widget::on_pbUploadNew_clicked()
{
    try2upload(false);
}

//-----------------------------------------------------------------------------------------

bool Widget::try2upload(const bool &all)
{
    Q_UNUSED(all);

    checkADirectoryAndACommand();
    if(ui->pbUploadAll->isEnabled() && ui->pbUploadNew->isEnabled()){

        myuploadstate.isUploading = true;
        checkADirectoryAndACommand();

        FileSearcher *f = new FileSearcher(getProjectDirectory(),
                                           getCreateAdirCommand(),
                                           getCommandText(),
                                           getCommandDirText(),
                                           ui->pteSubDirs->toPlainText().split("\n"),
                                           ui->ptePaths->toPlainText().split("\n"));
        QThread *t = new QThread(this);
        f->moveToThread(t);

        connect(f, SIGNAL(destroyed(QObject*)), t, SLOT(quit()));
        connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
        connect(t, SIGNAL(started()), f, SLOT(onThreadStarted()));

        connect(f, SIGNAL(append2log(QString)), this, SLOT(append2log(QString)));
        connect(f, SIGNAL(onUploadingDone()), this, SLOT(onUploadingFinished()));
        t->start();

        return true;
    }
    append2log(tr("Couldn't do the operation"));
    return false;
}

//-----------------------------------------------------------------------------------------

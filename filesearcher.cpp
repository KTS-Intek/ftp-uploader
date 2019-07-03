#include "filesearcher.h"

//-----------------------------------------------------------------------------------------------------------------

FileSearcher::FileSearcher(const QString &projectdir, const QString &createAremoteDirCommand, const QString &uploadcommand, const QString &uploadcommanddir,
                           const QStringList &subdirs, const QStringList &paths, QObject *parent) : QObject(parent)
{
    mysett.createAremoteDirCommand = createAremoteDirCommand;
    mysett.projectdir = projectdir;
    mysett.uploadcommand = uploadcommand;
    mysett.uploadcommanddir = uploadcommanddir;
    mysett.subdirs = subdirs;
    mysett.paths = paths;
}

//-----------------------------------------------------------------------------------------------------------------

bool FileSearcher::uploadTheFile(const QString &absolutefilename, const QString &uploadCommand, const QString &appname, QStringList &errors)
{
    emit append2log(tr("Uploading '%1'").arg(absolutefilename));


    return launchBashProcess(uploadCommand.arg(absolutefilename).arg(appname), errors);
}

//-----------------------------------------------------------------------------------------------------------------

bool FileSearcher::uploadTheSubdir(const QString &absolutedirname, const QString &uploadCommandDir, const QString &appname, QStringList &errors)
{
    emit append2log(tr("Uploading '%1'").arg(absolutedirname));
    return launchBashProcess(uploadCommandDir.arg(absolutedirname).arg(appname), errors);
}

//-----------------------------------------------------------------------------------------------------------------

bool FileSearcher::launchBashProcess(const QString &command2bash, QStringList &errors)
{
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start("bash");

    if(!proc.waitForStarted(1111)){
        errors.append(tr("Couldn't launch bash, %1").arg(proc.errorString()));
        return false;
    }

    proc.write(QString("%1 && exit\n").arg(command2bash).toUtf8());
    proc.waitForBytesWritten();
    QTime time;
    time.start();
    QString readarr;
    for(int i = 0, msecmax = 3600 * 1000; i < 0xFFFFFF && time.elapsed() < msecmax; i++ ){
        if(proc.waitForReadyRead(1000))
            readarr.append(proc.readAll());
        if(proc.state() != QProcess::Running){
            break;
        }
    }
    errors.append(readarr);
//    if(command2bash.startsWith("scp ") ){
//        return (readarr.contains("100%"));
//    }
    return true;
}

//-----------------------------------------------------------------------------------------------------------------

FileSearcher::UploadFileAndSubDirs FileSearcher::getFilesAndSubdirs(const QString &absolutedirname, const QStringList &paths, const QStringList &usersubdirs, const quint8 &depth)
{
    UploadFileAndSubDirs filesDirs;
    if(depth > 10)
        return filesDirs;

    QDir dir(absolutedirname);
    const QFileInfoList files = dir.entryInfoList(QDir::Files|QDir::Executable|QDir::Readable);


    for(int i = 0, imax = files.size(); i < imax; i++){

        const QString filename = files.at(i).fileName();
        if(!paths.contains(filename))
            continue;        


        const QStringList subdirs = dir.entryList(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot);
        QStringList appsubdirs;
        for(int j = 0, jmax = subdirs.size(); j < jmax; j++){
            if(usersubdirs.contains(subdirs.at(j)))
                appsubdirs.append(QString("%1/%2").arg(absolutedirname).arg(subdirs.at(j)));
        }
        if(!appsubdirs.isEmpty())
            filesDirs.filename2subdirs.insert(filename, appsubdirs);

        filesDirs.listfiles.append(files.at(i).absoluteFilePath());
        filesDirs.filename2bithtime.insert(filename, files.at(i).lastModified());
        filesDirs.filenames.append(filename);
    }


    const QStringList subdirs = dir.entryList(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot);
    for(int j = 0, jmax = subdirs.size(); j < jmax; j++){
        if(usersubdirs.contains(subdirs.at(j)))
            continue;

        const UploadFileAndSubDirs nextlevel = getFilesAndSubdirs(QString("%1/%2").arg(absolutedirname).arg(subdirs.at(j)), paths, usersubdirs, depth + 1);
        if(nextlevel.listfiles.isEmpty())
            continue;
        for(int k = 0, kmax = nextlevel.listfiles.size(); k < kmax; k++){
            const QString filename = nextlevel.filenames.at(k);
            if(filesDirs.filename2bithtime.contains(filename)){
                const QDateTime currfile = filesDirs.filename2bithtime.value(filename);
                const QDateTime nextfile = nextlevel.filename2bithtime.value(filename);

                if(currfile.isValid() && nextfile.isValid() && currfile > nextfile)
                    continue;

            }
            filesDirs.listfiles.append(nextlevel.listfiles.at(k));
            filesDirs.filename2bithtime.insert(filename, nextlevel.filename2bithtime.value(filename));
            filesDirs.filename2subdirs.insert(filename, nextlevel.filename2subdirs.value(filename));
            filesDirs.filenames.append(filename);
        }


    }
    return filesDirs;

}

//-----------------------------------------------------------------------------------------------------------------

void FileSearcher::onThreadStarted()
{
    emit append2log(tr("Looking for files"));
    try2uploadSomething();
    emit onUploadingDone();
    QTimer::singleShot(111, this, SLOT(deleteLater()));
}

//-----------------------------------------------------------------------------------------------------------------

void FileSearcher::try2uploadSomething()
{
    const UploadFileAndSubDirs filesAndDirs = getFilesAndSubdirs(mysett.projectdir, mysett.paths, mysett.subdirs, 0);

    if(filesAndDirs.listfiles.isEmpty() ){
        emit append2log(tr("Noting to upload"));
        return;
    }
    emit append2log(tr("Query:\n%1\n").arg(filesAndDirs.listfiles.join("\n")));

    for(int i = 0, imax = filesAndDirs.listfiles.size(); i < imax; i++){
        QStringList errs;

        if(!launchBashProcess(mysett.createAremoteDirCommand.arg(filesAndDirs.filenames.at(i)), errs)){
            emit append2log(tr("Couldn't create a directory '%1', %2").arg(filesAndDirs.filenames.at(i)).arg(errs.join("\n")));
            continue;
        }

        if(uploadTheFile(filesAndDirs.listfiles.at(i), mysett.uploadcommand, filesAndDirs.filenames.at(i), errs)){
            emit append2log(tr("File was uploaded '%1', %2").arg(filesAndDirs.listfiles.at(i)).arg(errs.join("\n")));

            errs.clear();
            const QStringList subdirs = filesAndDirs.filename2subdirs.value(filesAndDirs.filenames.at(i));
            for(int j = 0, jmax = subdirs.size(); j < jmax; j++){
                if(uploadTheSubdir(subdirs.at(j), mysett.uploadcommanddir, filesAndDirs.filenames.at(i), errs)){
                    emit append2log(tr("Directory was uploaded '%1', %2").arg(subdirs.at(j)).arg(errs.join("\n")));
                }else{
                    emit append2log(tr("Couldn't upload '%1', %2").arg(subdirs.at(j)).arg(errs.join("\n")));
                }
            }

        }else{
            emit append2log(tr("Couldn't upload '%1', %2").arg(filesAndDirs.listfiles.at(i)).arg(errs.join("\n")));
        }


    }


}

//-----------------------------------------------------------------------------------------------------------------

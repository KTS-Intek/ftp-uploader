#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include <QObject>
#include <QtCore>

class FileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit FileSearcher(const QString &projectdir, const QString &createAremoteDirCommand,
                          const QString &uploadcommand, const QString &uploadcommanddir, const QStringList &subdirs, const QStringList &paths, QObject *parent = nullptr);

    struct ItIsMySett
    {
        QString projectdir;
        QString createAremoteDirCommand;
        QString uploadcommand;
        QString uploadcommanddir;
        QStringList subdirs;//lang plugin2
        QStringList paths;

        ItIsMySett() {}
    } mysett;

    struct UploadFileAndSubDirs
    {
        QStringList listfiles;//absolute path
        QMap<QString,QDateTime> filename2bithtime;
        QMap<QString,QStringList> filename2subdirs;
        QStringList filenames;//just filename
        UploadFileAndSubDirs() {}
    };

    bool uploadTheFile(const QString &absolutefilename, const QString &uploadCommand, const QString &appname, QStringList &errors);

    bool uploadTheSubdir(const QString &absolutedirname, const QString &uploadCommandDir, const QString &appname, QStringList &errors);

    bool launchBashProcess(const QString &command2bash, QStringList &errors);

    UploadFileAndSubDirs getFilesAndSubdirs(const QString &absolutedirname, const QStringList &paths, const QStringList &usersubdirs, const quint8 &depth);


signals:
    void append2log(QString message);
    void onUploadingDone();

public slots:
    void onThreadStarted();

    void try2uploadSomething();



};

#endif // FILESEARCHER_H

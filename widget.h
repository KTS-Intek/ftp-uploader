#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtCore>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    struct UploadState
    {
        bool isUploading;
        bool closeAppOnFinish;
        UploadState() : isUploading(false), closeAppOnFinish(false) {}
    } myuploadstate;

    QString getSettingsFileName() const;

    QString getCommandText() const;

    QString getCommandDirText() const;

    QString getProjectDirectory() const;

    QString getCreateAdirCommand() const;

    QString showHelp() const;


public slots:
    void reloadSettings();

    void saveSettings();

    void checkADirectoryAndACommand();

    void checkAppArguments();

    void append2log(const QString &ptetext);


    void onUploadingFinished();

private slots:
    void on_pbReloadSettings_clicked();

    void on_pbSaveSettings_clicked();

    void on_toolButton_clicked();

    void on_pbUploadAll_clicked();

    void on_pbUploadNew_clicked();

private:
    Ui::Widget *ui;

    bool try2upload(const bool &all);


};

#endif // WIDGET_H

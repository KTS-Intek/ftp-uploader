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

    QString getCommandText() const;

    QString getProjectDirectory() const;

public slots:
    void reloadSettings();

    void saveSettings();

    void checkADirectoryAndACommand();

private slots:
    void on_pbReloadSettings_clicked();

    void on_pbSaveSettings_clicked();

    void on_toolButton_clicked();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H

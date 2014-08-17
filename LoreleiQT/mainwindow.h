#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <lorelei.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_enterButton_clicked();

private:
    Ui::MainWindow *ui;
    Lorelei lora;
};

#endif // MAINWINDOW_H

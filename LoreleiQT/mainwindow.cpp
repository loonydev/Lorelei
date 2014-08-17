#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->loreleiInput, SIGNAL(returnPressed()),ui->enterButton,SIGNAL(clicked()));

    //Инициализируем Лору
    lora.SetDebugPrinter([&](const string &str){
        ui->loreleiOutput->setText(ui->loreleiOutput->toPlainText() + QString::fromStdString(str));
    });
    lora.SetErrorPrinter([&](const string &str){
        ui->loreleiOutput->setHtml(ui->loreleiOutput->toHtml() +
                                   QString("<span style=\" color:#ff0000;\">") + QString::fromStdString(str) + "</span>");
    });

    lora.Init();

    ui->dbPathLabel->setText(QString::fromStdString(lora.GetDBPath()));
    ui->loreleiVersionLabel->setText(QString::fromStdString(lora.GetVersion()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_enterButton_clicked()
{
    string request = ui->loreleiInput->text().toLower().toStdString();

    lora.Parse(request);

    lora.PrintResults();

    lora.ClearCommands();
}

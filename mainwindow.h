#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QTextStream>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_addMotorButton_clicked();
    void on_removeMotorButton_clicked();
    void on_motorList_itemPressed(QListWidgetItem *item);
    void on_saveButton_clicked();
    void on_exportMotButton_clicked();

    void on_addMapSetButton_clicked();
    void on_removeMapSetButton_clicked();
    void on_mapList_itemPressed(QListWidgetItem *item);
    void on_addMapButton_clicked();
    void on_removeMapButton_clicked();
    void on_saveMapSetButton_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_currentMapCombo_currentIndexChanged(int index);
    void on_saveMapButton_clicked();
    void on_exportMapButton_clicked();

    void on_dbPathButton_clicked();
    void on_saveSettingsButton_clicked();

private:
    //Database
    static QSqlDatabase database;

    void loadMotorList();
    void loadMapSetList();
    void loadMapSetMaps(int mapsetCod);
    void loadDb(QString dbPath);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

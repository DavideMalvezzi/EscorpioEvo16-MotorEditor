#include "mainwindow.h"
#include "ui_mainwindow.h"

QSqlDatabase MainWindow::database = QSqlDatabase::addDatabase("QSQLITE", "MotorDB");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    QSettings settings("CANAnalyzer.ini", QSettings::IniFormat);
    QString dbPath = settings.value("Database/path", "").toString();

    if(!dbPath.isEmpty()){
       ui->dbPathText->setText(dbPath);
       loadDb(dbPath);
    }
    else{
        QMessageBox::critical(this, "Error", "Invalid database path!");
    }

    ui->torqueText->setValidator(new QDoubleValidator);
    ui->speedText->setValidator(new QDoubleValidator);
    ui->speedTorqueText->setValidator(new QDoubleValidator);
    ui->frictionText->setValidator(new QDoubleValidator);
    ui->frictionGradText->setValidator(new QDoubleValidator);
    ui->gearTrainText->setValidator(new QDoubleValidator);

    ui->a0Text->setValidator(new QDoubleValidator);
    ui->a1Text->setValidator(new QDoubleValidator);
    ui->a2Text->setValidator(new QDoubleValidator);
    ui->synchItrhText->setValidator(new QDoubleValidator);
    ui->flatLevText->setValidator(new QDoubleValidator);
    ui->maxSpeedText->setValidator(new QDoubleValidator);
}

void MainWindow::on_tabWidget_currentChanged(int index){
    loadMotorList();
    loadMapSetList();

    ui->motorPropGroup->setEnabled(false);
    ui->mapPropGroup->setEnabled(false);
    ui->mapSetPropGroup->setEnabled(false);

}

MainWindow::~MainWindow(){
    delete ui;
}

//Motor
void MainWindow::loadMotorList(){
    QSqlQuery q(database);
    QListWidgetItem* item;
    q.exec("SELECT ID, Name FROM Motor");
    ui->motorList->clear();

    while(q.next()){
        item = new QListWidgetItem(q.value(1).toString(), ui->motorList);
        item->setData(Qt::UserRole, q.value(0).toInt());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        ui->motorList->addItem(item);
    }

//    if(ui->motorList->count() <= 0){
//        ui->motorPropGroup->setEnabled(false);
//    }
//    else{
//        ui->motorPropGroup->setEnabled(true);
//    }

}

void MainWindow::on_addMotorButton_clicked(){
    QSqlQuery q(database);
    q.exec("INSERT INTO Motor DEFAULT VALUES");
    loadMotorList();
}

void MainWindow::on_removeMotorButton_clicked(){
    QSqlQuery q(database);
    if(ui->motorList->currentRow() != -1){
        q.prepare("DELETE FROM Motor WHERE ID = :id");
        q.bindValue(":id", ui->motorList->currentItem()->data(Qt::UserRole).toInt());
        q.exec();
        loadMotorList();
    }
}

void MainWindow::on_motorList_itemPressed(QListWidgetItem *item){
    QSqlQuery q(database);
    q.prepare("SELECT * FROM Motor WHERE ID = :id");
    q.bindValue(":id", item->data(Qt::UserRole).toInt());
    q.exec();
    q.next();

    ui->motorNameText->setText(q.value(1).toString());
    ui->defMapSpin->setValue(q.value(2).toInt());
    ui->torqueText->setText(q.value(3).toString());
    ui->speedText->setText(q.value(4).toString());
    ui->speedTorqueText->setText(q.value(5).toString());
    ui->frictionText->setText(q.value(6).toString());
    ui->frictionGradText->setText(q.value(7).toString());
    ui->gearTrainText->setText(q.value(8).toString());
    ui->commentText->setPlainText(q.value(9).toString());

    ui->motorPropGroup->setEnabled(true);
}

void MainWindow::on_saveButton_clicked(){
    int index = ui->motorList->currentRow();

    if(index != -1){
        QSqlQuery q(database);
        q.prepare("UPDATE Motor SET "
                  "Name = :name, "
                  "DefaultMap = :map, "
                  "TorqueConst = :torque, "
                  "SpeedConst = :speed, "
                  "SpeedTorqueGrad = :sp, "
                  "Friction = :friction, "
                  "FrictionGrad = :frgrad, "
                  "GearTrain = :geart, "
                  "Comment = :comment "
                  "WHERE ID = :id");
        q.bindValue(":id", ui->motorList->currentItem()->data(Qt::UserRole).toInt());
        q.bindValue(":name", ui->motorNameText->text());
        q.bindValue(":map", ui->defMapSpin->value());
        q.bindValue(":torque", ui->torqueText->text());
        q.bindValue(":speed", ui->speedText->text());
        q.bindValue(":sp", ui->speedTorqueText->text());
        q.bindValue(":friction", ui->frictionText->text());
        q.bindValue(":frgrad", ui->frictionGradText->text());
        q.bindValue(":geart", ui->gearTrainText->text());
        q.bindValue(":comment", ui->commentText->toPlainText());
        q.exec();

        if(q.lastError().isValid()){
            QMessageBox::critical(this, "Error", q.lastError().text());
        }

        loadMotorList();

        ui->motorList->setCurrentRow(index);
    }
}

void MainWindow::on_exportMotButton_clicked(){
    #define MOTOR_PROP_COUNT 8
    QListWidgetItem* item;
    QFileDialog d(this);
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setOption(QFileDialog::ShowDirsOnly);
    d.setFileMode(QFileDialog::Directory);
    if(d.exec()){
        QFile file(d.selectedFiles().first().append("/MOTOR.CFG"));
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            QSqlQuery q(MainWindow::database);
            int count = 0;

            for(int i = 0; i < ui->motorList->count(); i++){
                item = ui->motorList->item(i);
                if(item->checkState() == Qt::Checked){
                    count++;
                }
            }

            stream<<"PROPS=" << count * MOTOR_PROP_COUNT << "\r\n" << "\r\n";


            for(int i = 0; i < ui->motorList->count(); i++){
                item = ui->motorList->item(i);
                if(item->checkState() == Qt::Checked){
                    q.prepare("SELECT * FROM Motor WHERE ID = :id");
                    q.bindValue(":id", item->data(Qt::UserRole).toInt());
                    q.exec();
                    q.next();

                    stream << "NAME=" << q.value(1).toString() << "\r\n";
                    stream << "DEF_MAP=" << q.value(2).toString() << "\r\n";
                    stream << "TORQUE_CONST=" << q.value(3).toString() << "\r\n";
                    stream << "SPEED_CONST=" << q.value(4).toString() << "\r\n";
                    stream << "SPD_TORQUE_GRAD=" << q.value(5).toString() << "\r\n";
                    stream << "FRICTION=" << q.value(6).toString() << "\r\n";
                    stream << "FRICTION_GRAD=" << q.value(7).toString() << "\r\n";
                    stream << "GEAR_TRAIN=" << q.value(8).toString() << "\r\n";
                    stream << "\r\n";
                }
            }

            file.close();
        }
    }
}

//MapSet
void MainWindow::loadMapSetList(){
    QSqlQuery q(database);
    QListWidgetItem* item;
    q.exec("SELECT * FROM Mapset");
    ui->mapList->clear();

    while(q.next()){
        item = new QListWidgetItem(q.value(1).toString(), ui->mapList);
        item->setData(Qt::UserRole, q.value(0).toInt());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        ui->mapList->addItem(item);
    }
}

void MainWindow::loadMapSetMaps(int mapsetCod){
    QSqlQuery q(database);
    QSqlQueryModel* mapModel = new QSqlQueryModel(this);
    q.prepare("SELECT ID, Name FROM Map WHERE CodMapset = :id");
    q.bindValue(":id", mapsetCod);
    q.exec();
    mapModel->setQuery(q);

    ui->currentMapCombo->clear();
    ui->currentMapCombo->setModel(mapModel);
    ui->currentMapCombo->setModelColumn(1);

    ui->mapSetPropGroup->setEnabled(true);
    if(ui->currentMapCombo->currentIndex() != -1){
        ui->mapPropGroup->setEnabled(true);
    }
    else{
        ui->mapPropGroup->setEnabled(false);
    }

}

void MainWindow::on_addMapSetButton_clicked(){
    QSqlQuery q(database);
    q.exec("INSERT INTO Mapset DEFAULT VALUES");
    loadMapSetList();
}

void MainWindow::on_removeMapSetButton_clicked(){
    QSqlQuery q(database);
    if(ui->mapList->currentRow() != -1){
        q.prepare("DELETE FROM Mapset WHERE ID = :id");
        q.bindValue(":id", ui->mapList->currentItem()->data(Qt::UserRole).toInt());
        q.exec();
        loadMapSetList();
    }
}

void MainWindow::on_mapList_itemPressed(QListWidgetItem *item){
    int motorCod;
    QSqlQuery q(database);
    q.prepare("SELECT * FROM Mapset WHERE ID = :id");
    q.bindValue(":id", item->data(Qt::UserRole).toInt());
    q.exec();
    q.next();

    ui->mapSetNameText->setText(q.value(1).toString());
    ui->mapSetCommentText->setPlainText(q.value(3).toString());
    motorCod = q.value(2).toInt();


    QSqlQueryModel* motorModel = new QSqlQueryModel(this);
    q.prepare("SELECT ID, Name FROM Motor");
    q.exec();
    motorModel->setQuery(q);

    ui->mapSetMotorCombo->clear();
    ui->mapSetMotorCombo->setModel(motorModel);
    ui->mapSetMotorCombo->setModelColumn(0);
    int index = ui->mapSetMotorCombo->findData(motorCod, Qt::DisplayRole);
    ui->mapSetMotorCombo->setModelColumn(1);
    ui->mapSetMotorCombo->setCurrentIndex(index);

    loadMapSetMaps(ui->mapList->currentItem()->data(Qt::UserRole).toInt());

}

void MainWindow::on_addMapButton_clicked(){
    if(ui->mapList->currentRow() != -1){
        QSqlQuery q(database);
        q.prepare("INSERT INTO Map(CodMapset) VALUES (:id)");
        q.bindValue(":id", ui->mapList->currentItem()->data(Qt::UserRole).toInt());
        q.exec();
        loadMapSetMaps(ui->mapList->currentItem()->data(Qt::UserRole).toInt());
    }
}

void MainWindow::on_removeMapButton_clicked(){
    if(ui->currentMapCombo->currentIndex() != -1){
        QSqlQueryModel* mapModel = (QSqlQueryModel*)ui->currentMapCombo->model();
        QSqlRecord record =  mapModel->record(ui->currentMapCombo->currentIndex());
        QSqlQuery q(database);
        q.prepare("DELETE FROM Map WHERE ID = :id");
        q.bindValue(":id", record.value(0).toInt());
        q.exec();

        loadMapSetMaps(ui->mapList->currentItem()->data(Qt::UserRole).toInt());
    }
}

void MainWindow::on_saveMapSetButton_clicked(){
    int index = ui->mapList->currentRow();
    if(index != -1){
        QSqlQueryModel* motorModel = (QSqlQueryModel*)ui->mapSetMotorCombo->model();
        int codMotor = motorModel->record(ui->mapSetMotorCombo->currentIndex()).value(0).toInt();
        QSqlQuery q(database);
        q.prepare("UPDATE Mapset SET Name = :name, CodMotor = :codM, Comment = :cmt WHERE ID = :id");
        q.bindValue(":id", ui->mapList->currentItem()->data(Qt::UserRole).toInt());
        q.bindValue(":name", ui->mapSetNameText->text());
        q.bindValue(":codM", codMotor);
        q.bindValue(":cmt", ui->mapSetCommentText->toPlainText());
        q.exec();

        if(q.lastError().isValid()){
            QMessageBox::critical(this, "Error", QString("Error on saving: %1").arg(q.lastError().text()));
        }
        else{
            loadMapSetList();
            ui->mapList->setCurrentRow(index);
        }
    }

}

void MainWindow::on_currentMapCombo_currentIndexChanged(int index){
    QSqlQuery q(database);
    if(index != -1){
        QSqlQueryModel* mapModel = (QSqlQueryModel*)ui->currentMapCombo->model();
        QSqlRecord record =  mapModel->record(ui->currentMapCombo->currentIndex());
        q.prepare("SELECT * FROM Map WHERE ID = :id");
        q.bindValue(":id", record.value(0).toInt());
        q.exec();
        q.next();

        ui->mapNameText->setText(q.value(1).toString());
        ui->a0Text->setText(q.value(2).toString());
        ui->a1Text->setText(q.value(3).toString());
        ui->a2Text->setText(q.value(4).toString());
        ui->synchItrhText->setText(q.value(5).toString());
        ui->flatLevText->setText(q.value(6).toString());
        ui->maxSpeedText->setText(q.value(7).toString());

        ui->flatOutCheck->setChecked(q.value(8).toBool());
        ui->useSynchRectCheck->setChecked(q.value(9).toBool());
        ui->useSynchSafeCheck->setChecked(q.value(10).toBool());
        ui->useEnRecoveryCheck->setChecked(q.value(11).toBool());
    }
}

void MainWindow::on_saveMapButton_clicked(){
   int index = ui->currentMapCombo->currentIndex();
   if(index != -1 && ui->mapList->currentRow() != -1){
       QSqlQuery q(database);
       QSqlQueryModel* mapModel = (QSqlQueryModel*)ui->currentMapCombo->model();
       QSqlRecord record =  mapModel->record(ui->currentMapCombo->currentIndex());
       q.prepare("UPDATE Map SET "
                 "Name = :name, "
                 "A0 = :a0, "
                 "A1 = :a1, "
                 "A2 = :a2, "
                 "SyncItrh = :syncI, "
                 "FlatLev = :flatL, "
                 "MaxSpeed = :maxS, "
                 "FlatOut = :flatO, "
                 "UseSyncRect = :syncR, "
                 "UseSyncSafe = :syncS, "
                 "UseEnergyRecovery = :enRec "
                 "WHERE ID = :id");
       q.bindValue(":id", record.value(0).toInt());
       q.bindValue(":name", ui->mapNameText->text());
       q.bindValue(":a0", ui->a0Text->text());
       q.bindValue(":a1", ui->a1Text->text());
       q.bindValue(":a2", ui->a2Text->text());
       q.bindValue(":syncI", ui->synchItrhText->text());
       q.bindValue(":flatL", ui->flatLevText->text());
       q.bindValue(":maxS", ui->maxSpeedText->text());

       q.bindValue(":flatO", ui->flatOutCheck->isChecked());
       q.bindValue(":syncR", ui->useSynchRectCheck->isChecked());
       q.bindValue(":syncS", ui->useSynchSafeCheck->isChecked());
       q.bindValue(":enRec", ui->useEnRecoveryCheck->isChecked());
       q.exec();

       loadMapSetMaps(ui->mapList->currentItem()->data(Qt::UserRole).toInt());
       ui->currentMapCombo->setCurrentIndex(index);
   }
}

void MainWindow::on_exportMapButton_clicked(){
    QListWidgetItem* item;
    QFileDialog d(this);
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setOption(QFileDialog::ShowDirsOnly);
    d.setFileMode(QFileDialog::Directory);
    if(d.exec()){
        QFile file(d.selectedFiles().first().append("/MAPSET.CFG"));
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            QSqlQuery q(MainWindow::database);
            int count = 0;

            for(int i = 0; i < ui->mapList->count(); i++){
                item = ui->mapList->item(i);
                if(item->checkState() == Qt::Checked){
                    count++;
                }
            }

            stream<<"PROPS=" << count << "\r\n" << "\r\n";


            for(int i = 0; i < ui->mapList->count(); i++){
                item = ui->mapList->item(i);
                if(item->checkState() == Qt::Checked){
                    q.prepare("SELECT * FROM Map WHERE CodMapset = :id");
                    q.bindValue(":id", item->data(Qt::UserRole).toInt());
                    q.exec();

                    stream << item->text() << "={" << "\r\n";
                    while(q.next()){
                        stream << "\tNAME=" << q.value(1).toString() << "\r\n";
                        stream << "\tA0=" << q.value(2).toString() << "\r\n";
                        stream << "\tA1=" << q.value(3).toString() << "\r\n";
                        stream << "\tA2=" << q.value(4).toString() << "\r\n";
                        stream << "\tSYNC_ITRH=" << q.value(5).toString() << "\r\n";
                        stream << "\tFLAT_LEV=" << q.value(6).toString() << "\r\n";
                        stream << "\tMAX_SPEED=" << q.value(7).toString() << "\r\n";
                        stream << "\tFLAT_OUT=" << q.value(8).toString() << "\r\n";
                        stream << "\tSYNC_RECT=" << q.value(9).toString() << "\r\n";
                        stream << "\tSYNC_SAFE=" << q.value(10).toString() << "\r\n";
                        stream << "\tEN_RECOVERY=" << q.value(11).toString() << "\r\n";
                        stream << "\r\n";
                    }

                    stream << "}" << "\r\n" << "\r\n";

                }
            }

            file.close();
        }
    }
}


//Settings
void MainWindow::loadDb(QString dbPath){
    database.setDatabaseName(dbPath);
    //Caricamento DB e creazione dialog delle impostazioni
    if(!database.open()){
      QMessageBox::critical(this, "Error", "Error on database opening");
    }else{
      QSqlQuery q(database);
      q.exec("PRAGMA foreign_keys=ON;");
      loadMotorList();
      loadMapSetList();
    }
}


void MainWindow::on_dbPathButton_clicked(){
    QString fileName = QFileDialog::getOpenFileName(this, "Select DB", "", "SQLite DB (*.db *.sql *.sqlite)");
    if(!fileName.isNull() && !fileName.isEmpty()){
        ui->dbPathText->setText(fileName);
        loadDb(fileName);
    }
}

void MainWindow::on_saveSettingsButton_clicked(){
    QSettings settings("CANAnalyzer.ini", QSettings::IniFormat);
    settings.beginGroup("Database");
        settings.setValue("path", ui->dbPathText->text());
    settings.endGroup();

}

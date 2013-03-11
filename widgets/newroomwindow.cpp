#include "newroomwindow.h"
#include "ui_newroomwindow.h"

NewRoomWindow::NewRoomWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewRoomWindow)
{
    ui->setupUi(this);
    // set validator for two lineinputs
    QIntValidator *inva = new QIntValidator(this);
    inva->setRange(1, 65535);
    ui->height_input->setValidator(inva);
    ui->width_input->setValidator(inva);
    ui->buttonBox->
            button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(ui->lineEdit, &QLineEdit::textChanged,
            this, &NewRoomWindow::onNameChanged);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok),
            &QPushButton::clicked,
            this, &NewRoomWindow::onOk);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel),
            &QPushButton::clicked,
            this, &NewRoomWindow::onCancel);
}

NewRoomWindow::~NewRoomWindow()
{
    delete ui;
}

void NewRoomWindow::onServerResponse(const QVariantMap &m)
{
    if(m["response"] != "newroom"
            || !m.contains("result"))
        return;
    if(m["result"].toBool()){
        // TODO
        ui->progressBar->setValue(100);
        QString msg = tr("Succeed!");
        QMessageBox::information(this, tr("Go get your room!"),
                                 msg,
                                 QMessageBox::Ok);
        if(m.contains("info")){
            QVariantMap info = m.value("info").toMap();
            //            int cmdPort = info.value("cmdPort").toInt();
            //            QString password = info.value("password").toString();
            QString key = info.value("key").toString();

            QCryptographicHash hash(QCryptographicHash::Md5);
            hash.addData(ui->lineEdit->text().toUtf8());
            QString hashed_name = hash.result().toHex();
            QSettings settings(GlobalDef::SETTINGS_NAME,
                               QSettings::defaultFormat(),
                               qApp);
            settings.setValue("rooms/"+hashed_name, key);
            settings.sync();
        }
        this->accept();
    }

    if(!m.contains("errcode")){
        return;
    }else{
        int errcode = m["errcode"].toInt();
        QString errmsg = tr("Error: ")+QString::number(errcode)+
                tr("\nDo you want to retry?");
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(this, tr("Error!"),
                                      errmsg,
                                      QMessageBox::Retry|
                                      QMessageBox::Abort);
        if(reply == QMessageBox::Retry){
            this->onOk();
        }else{
            return;
        }
    }
}

void NewRoomWindow::onOk()
{
    QString s = ui->lineEdit->text();
    if(s.isEmpty() ||
            s.length() > 16){
        QString errmsg = tr("Your room name is empty or too long!");
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(this, tr("Error!"),
                                      errmsg,
                                      QMessageBox::Abort);
        if(reply == QMessageBox::Abort){
            return;
        }
    }
    s = ui->plainTextEdit->toPlainText();
    if(s.length()>40){
        QString errmsg = tr("Your welcome message is too long!");
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(this, tr("Error!"),
                                      errmsg,
                                      QMessageBox::Abort);
        if(reply == QMessageBox::Abort){
            return;
        }
    }

    // TODO: due with size
    int width = ui->width_input->text().toInt();
    int height = ui->height_input->text().toInt();
    if(width > 3240 || height > 2160){
        QMessageBox::warning(this,
                             tr("Large Canvas!"),
                             tr("You set a large canvas size, "
                                "which may have high load on "
                                "low-memory computers."));
    }
    QVariantMap sizeMap;
    sizeMap.insert("width", width);
    sizeMap.insert("height", height);


    QString pw;
    if(ui->checkBox->isChecked()){
        pw = ui->lineEdit_2->text();
    }

    QVariantMap map;
    map.insert("name", ui->lineEdit->text());
    map.insert("welcomemsg", ui->plainTextEdit->toPlainText());
    map.insert("maxload", ui->spinBox->value());
    map.insert("password", pw);
    map.insert("emptyclose", ui->checkBox_2->isChecked());
    map.insert("size", QVariant(sizeMap));

    ui->progressBar->setValue(30);
    emit newRoom(map);
}

void NewRoomWindow::onCancel()
{
    QMessageBox::StandardButton reply;
    QString msg = tr("Do you really want to cancel?");
    reply = QMessageBox::question(this, tr("Cancel?"),
                                  msg,
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        this->reject();
}

void NewRoomWindow::onNameChanged(const QString &name)
{
    ui->progressBar->setValue(0);
    if(!name.isEmpty()){
        ui->buttonBox->
                button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void NewRoomWindow::disableEdit(bool t)
{
    this->setDisabled(t);
    ui->buttonBox->setEnabled(true);
}

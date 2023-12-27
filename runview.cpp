#include "runview.h"
#include "ui_runview.h"
#include <fstream>
#include <iostream>
#include <json/config.h>
#include <json/json.h>
#include <json/reader.h>
#include <QFile>

#define CONFIG_JSON "/root/run_config.json"

using namespace std;
int cfg_isSavePic=1;
int cfg_abDelay=583;
int cfg_cdDelay=1705;
int cfg_coverWidth=100;
int cfg_ironWidth=8;

int Runview::JsonIORead(QString path)
{
        Json::Reader reader;
        Json::Value root;
        std::ifstream in(path.toLatin1(), std::ios::binary);

        if (reader.parse(in, root))
        {
            Json::Value devalue=root["runConfig"];

            if (!devalue.empty())
            {
                cfg_isSavePic=devalue["isSavePic"].asInt();
                cfg_abDelay=devalue["abDelay"].asInt();
                cfg_cdDelay=devalue["cdDelay"].asInt();
                cfg_coverWidth=devalue["coverWidth"].asInt();
                cfg_ironWidth=devalue["ironWidth"].asInt();
            }

        }

        ui->comboBox->setCurrentIndex(cfg_isSavePic);
        ui->lineEdit->setText(QString::number(cfg_abDelay));
        ui->lineEdit_2->setText(QString::number(cfg_cdDelay));
        ui->lineEdit_3->setText(QString::number(cfg_coverWidth));
        ui->lineEdit_4->setText(QString::number(cfg_ironWidth));

    return 0;
}

Runview::Runview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Runview)
{
    ui->setupUi(this);
    ui->comboBox->addItem("不保存图");
    ui->comboBox->addItem("保存图");
    ui->comboBox->addItem("只保存NG图");
    JsonIORead(CONFIG_JSON);

    connect(ui->okButton, &QPushButton::clicked, this, &Runview::on_pushButton_clicked);
}

Runview::~Runview()
{
    delete ui;
}

int Runview::JsonIOWrite(QString path)
{
    printf("start write json_path\n");

    if (ui->lineEdit->text().compare("") == 0 || ui->lineEdit_2->text().compare("") == 0
            || ui->lineEdit_3->text().compare("") == 0 || ui->lineEdit_4->text().compare("") == 0){
        QMessageBox mgBox(QMessageBox::Critical, "错误", "参数不可为0!!", QMessageBox::Yes | QMessageBox::No, this);
        mgBox.setFixedSize(400, 300);
        mgBox.exec();
        return -1;
    }

    cfg_isSavePic=ui->comboBox->currentIndex();
    cfg_abDelay=ui->lineEdit->text().toInt();
    cfg_cdDelay=ui->lineEdit_2->text().toInt();
    cfg_coverWidth=ui->lineEdit_3->text().toInt();
    cfg_ironWidth=ui->lineEdit_4->text().toInt();

    Json::Value root;
    Json::Value run;
    // 组装json内容
    run["isSavePic"]  = cfg_isSavePic;
    run["abDelay"] = cfg_abDelay;
    run["cdDelay"] = cfg_cdDelay;
    run["coverWidth"] = cfg_coverWidth;
    run["ironWidth"] = cfg_ironWidth;

    root["runConfig"] = run;
    // 将json内容（缩进格式）输出到文件
    Json::StyledWriter writer;
    ofstream os;
    os.open(path.toLatin1());
    os << writer.write(root);
    os.close();

    printf("end write json_path: success!  %s\n",path.toLatin1().data());
    return 0;
}

void Runview::on_pushButton_clicked()
{
    cout<<"enter confirm "<<endl;
    JsonIOWrite(CONFIG_JSON);
    ui->statusLabel->setText("<h2><font color=red>保存成功</font></h2>");
}

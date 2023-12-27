#include "quickview.h"
#include "ui_quickview.h"

#include <fstream>
#include <iostream>
#include <json/config.h>
#include <json/json.h>
#include <json/reader.h>
#include <QFile>
#include <QuickSetThread.h>

#define CONFIG_JSON "/home/q/Desktop/quick_config.json"

using namespace std;
float cfg_coverHigh=21.4;
float cfg_beltRate=350;

extern int cfg_abDelay;
extern int cfg_cdDelay;
extern int cfg_coverWidth;
extern int cfg_ironWidth;

extern int cfg_innerFreq1;
extern int cfg_outputNum1;

extern int cfg_innerFreq2;
extern int cfg_outputNum2;
extern int cfg_isSavePic;

//extern quectel_infer;
int Quickview::JsonIORead(QString path)
{
        Json::Reader reader;
        Json::Value root;
        std::ifstream in(path.toLatin1(), std::ios::binary);

        if (reader.parse(in, root))
        {
                Json::Value devalue=root["quickConfig"];

                cfg_coverHigh=devalue["coverHigh"].asFloat();
                cfg_beltRate=devalue["beltRate"].asFloat();
                cout<<cfg_coverHigh<<endl;
        }

        ui->lineEdit_1->setText(QString::number(cfg_beltRate,'f',0));
        ui->lineEdit_2->setText(QString::number(cfg_coverHigh,'f',1));

    return 0;
}


void Quickview::showMsg(QString str){
    ui->textEdit->append(str);

}

Quickview::Quickview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Quickview)
{
    ui->setupUi(this);
    JsonIORead(CONFIG_JSON);
    connect(ui->okButton, &QPushButton::clicked, this, &Quickview::on_pushButton_clicked);

    qs = new QuickSetThread(this);
    //    connect(this, &VideoPanel::emit_infer_mem, algoThread, &AlgoThread::transform);
    connect(qs,&QuickSetThread::sendMsg,this,&Quickview::showMsg);


}

Quickview::~Quickview()
{
    delete ui;
}

int Quickview::JsonIOWrite(QString path)
{
    printf("start write json_path\n");

    if (ui->lineEdit_1->text().compare("") == 0 || ui->lineEdit_2->text().compare("") == 0){
        QMessageBox mgBox(QMessageBox::Critical, "错误", "参数不可为0!!", QMessageBox::Yes | QMessageBox::No, this);
        mgBox.setFixedSize(400, 300);
        mgBox.exec();
        return -1;
    }

    cfg_beltRate=ui->lineEdit_1->text().toFloat();
    cfg_coverHigh=ui->lineEdit_2->text().toFloat();

    Json::Value root;
    Json::Value run;
    // 组装json内容
    run["beltRate"]  = cfg_beltRate;
    run["coverHigh"] = cfg_coverHigh;

    root["quickConfig"] = run;
    // 将json内容（缩进格式）输出到文件
    Json::StyledWriter writer;
    ofstream os;
    os.open(path.toLatin1());
    os << writer.write(root);
    os.close();

    printf("end write json_path: success!  %s\n",path.toLatin1().data());
    return 0;
}



void Quickview::on_pushButton_clicked()
{
    cout<<"enter confirm "<<endl;
    JsonIOWrite(CONFIG_JSON);
    QString result="";
    ui->textEdit->append("请等待参数设置......");

    qs->start();

}

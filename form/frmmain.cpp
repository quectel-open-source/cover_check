#include "frmmain.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"
#include "frmvideopanel.h"
#include "frmdbpage.h"
#include "settingsview.h"
#include <QHostAddress>
#include "DaHengCamera/GxIAPI.h"
#include "DaHengCamera/DxImageProc.h"
#include "MvCameraControl.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "qdebug.h"

//#include <QtXlsx>
#include "frmmain.h"
#include "appinit.h"
#include "quihelper.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "videobox.h"
#include "DaHengCamera/GxIAPI.h"
#include "DaHengCamera/DxImageProc.h"
#include "MvCamera.h"
#include <unistd.h>
#include <QTcpSocket>


#define SERVER_IP_ADDRESS  "127.0.0.1"
#define BAND_IP_ADDRESS    "127.0.0.1"
#define BAND_IP_PORT 8882
#define SOCKET_FILE "/home/q/1.jpg"
//extern  QTcpSocket *qlsocket;
#include "ModbusTcpClient.h"
//extern  ModbusTcpClient *modbusTcpClient;
extern QSqlQuery query;
extern void clearAll();

QString statusStr;
//extern int _numUsedMem[4];
extern int pushNumA;
extern int pushNumB;

//extern int findFast;
extern int _numOK;
extern int _numNG;
extern int _numTotal;
extern int _numOil;
extern int _numScratch;
extern int _numCrush;
extern int _numSwell;
extern int gpioNG;
extern int serialNum;
extern int ironWidth;
extern int longNum;
extern int batchNum;
extern int lostA;
//QXlsx::Document xlsx;
bool isSuspend=false;

frmMain::frmMain(QWidget *parent) : QDialog(parent), ui(new Ui::frmMain)
{
        checkng=0;
        checkok=0;
        checktotall=0;
    this->setWindowIcon(QIcon(":/uimain/icon/m1.png"));
    ui->setupUi(this);
    this->initForm();
}

frmMain::~frmMain()
{
    delete ui;
}



bool frmMain::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched == ui->widgetTitle) {
            //on_btnMenu_Max_clicked();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

extern QString enterDay;
void frmMain::initForm()
{
    this->setWindowTitle("AI");
    this->setWindowIcon(QIcon(":/uimain/icon/m1.png"));
    statusStr="状态正常";
    ui->widgetTitle->installEventFilter(this);
    ui->widgetTitle->setProperty("清空数据", "title");
    ui->widgetTop->setProperty("nav", "top");

    ui->labTitle1->setPixmap(QPixmap(":/uimain/icon/u8.png"));
    ui->labTitle1->setScaledContents(1);
    ui->labTitle1->setScaledContents(1);

    QIcon icon1;
    icon1.addFile(QStringLiteral(":/uimain/icon/u22.png"), QSize(), QIcon::Normal, QIcon::Off);
    ui->btnMenu_refresh->setIcon(icon1);
    QIcon icon2;
    icon1.addFile(QStringLiteral(":/uimain/icon/u21.png"), QSize(), QIcon::Normal, QIcon::Off);
    ui->btnMenu_setting->setIcon(icon1);

    label =  new QLabel();
    QLabel* label2 = new QLabel();
    label2->setFixedSize(20,20);
    label->setFixedSize(55,25);
    label2->setStyleSheet(QString("border:0px solid write;"));
    label->setStyleSheet(QString("border:0px solid write;"));
    QPixmap pix=QPixmap(":/uimain/icon/u17.png");
    QSize size = label2->size();
    pix.scaled(size,Qt::KeepAspectRatioByExpanding);
    label2->setPixmap(pix);
    label2->setScaledContents(true);
    label->setText("暂停");
    label->setStyleSheet("color:white");
    label->setScaledContents(true);
    QHBoxLayout* myLayout = new QHBoxLayout();
    myLayout->addSpacing(1);
    myLayout->addWidget(label2);
    myLayout->addSpacing(1);
    myLayout->addWidget(label);
    myLayout->addStretch();
    ui->btnstart->setStyleSheet("background-color:rgb(28,189,127);");
    ui->btnstart->setLayout(myLayout);

    QLabel* labelsql1 = new QLabel();
    QLabel* labelsql2 = new QLabel();
    labelsql2->setFixedSize(20,20);
    labelsql1->setFixedSize(55,25);
    labelsql2->setStyleSheet(QString("border:0px solid write;"));
    labelsql1->setStyleSheet(QString("border:0px solid write;"));
    QPixmap pixsql=QPixmap(":/uimain/icon/u20.png");
    QSize sqlsize = labelsql2->size();
    pixsql.scaled(sqlsize,Qt::KeepAspectRatioByExpanding);
    labelsql2->setPixmap(pixsql);
    labelsql2->setScaledContents(true);
    labelsql1->setText("清空状态");
    labelsql1->setStyleSheet("color:white");
    labelsql1->setScaledContents(true);
    QHBoxLayout* sqlLayout = new QHBoxLayout();
    sqlLayout->addSpacing(1);
    sqlLayout->addWidget(labelsql2);
    sqlLayout->addSpacing(1);
    sqlLayout->addWidget(labelsql1);
    sqlLayout->addStretch();
    ui->btnsql->setStyleSheet("background-color:rgb(248,173,23);");
    ui->btnsql->setLayout(sqlLayout);
    ui->widgetTitle->setStyleSheet("background-color:rgb(2,6,13);");

    // btnNg and btntotal start
    QFont ft;
    ft.setPointSize(15);

    QLabel* labelbatch1 = new QLabel();
    labelbatch2 = new QLabel();
    labelbatch1->setText("当前批次");
    labelbatch1->setStyleSheet("color:white; font-size: 15px;");
    labelbatch2->setText("0");
    labelbatch2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labelng1 = new QLabel();
    labelng2 = new QLabel();
    labelng1->setStyleSheet(QString("border:0px solid write;"));
    labelng2->setStyleSheet(QString("border:0px solid write;"));

    labelng1->setText("NG数量");
    labelng1->setStyleSheet("color:white; font-size: 15px;");
    //labelng1->setFixedWidth(100);

    labelng2->setText("0");
    labelng2->setStyleSheet("color:white; font-size: 15px;");
    //labelng2->setFixedWidth(90);
    //labelng1->setFont(ft);
    //labelng2->setFont(ft);
    QLabel* labelserial1 = new QLabel();
    labelserial2 = new QLabel();
    labelserial1->setText("串口到位");
    labelserial1->setStyleSheet("color:white; font-size: 15px;");
    labelserial2->setText("0");
    labelserial2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labellong1 = new QLabel();
    labellong2 = new QLabel();
    labellong1->setText("超长个数");
    labellong1->setStyleSheet("color:white; font-size: 15px;");
    labellong2->setText("0");
    labellong2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labelpush1 = new QLabel();
    labelpush2 = new QLabel();
    labelpush1->setText("相机到位");
    labelpush1->setStyleSheet("color:white; font-size: 15px;");
    labelpush2->setText("0");
    labelpush2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labeloil1 = new QLabel();
    labeloil2 = new QLabel();
    labeloil1->setText("油污");
    labeloil1->setStyleSheet("color:white; font-size: 15px;");
    labeloil2->setText("0%");
    labeloil2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labelcrush1 = new QLabel();
        labelcrush2 = new QLabel();
        labelcrush1->setText("压伤");
        labelcrush1->setStyleSheet("color:white; font-size: 15px;");
        labelcrush2->setText("0%");
        labelcrush2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labelswell1 = new QLabel();
        labelswell2 = new QLabel();
        labelswell1->setText("翘边");
        labelswell1->setStyleSheet("color:white; font-size: 15px;");
        labelswell2->setText("0%");
        labelswell2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labelscratch1 = new QLabel();
        labelscratch2 = new QLabel();
        labelscratch1->setText("划痕");
        labelscratch1->setStyleSheet("color:white; font-size: 15px;");
        labelscratch2->setText("0%");
        labelscratch2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labeliron1 = new QLabel();
            labeliron2 = new QLabel();
            labeliron1->setText("铁屑总长");
            labeliron1->setStyleSheet("color:white; font-size: 15px;");
            labeliron2->setText("0");
            labeliron2->setStyleSheet("color:white; font-size: 15px;");

    QLabel* labellosta1 = new QLabel();
            labellosta2 = new QLabel();
            labellosta1->setText("A面错位");
            labellosta1->setStyleSheet("color:white; font-size: 15px;");
            labellosta2->setText("0");
            labellosta2->setStyleSheet("color:white; font-size: 15px;");


    QGridLayout* ngLayout = new QGridLayout;
    ngLayout->addWidget(labelng1, 0, 0);
    ngLayout->addWidget(labelng2,0,1);
    ngLayout->addWidget(labeloil1, 1, 0);
    ngLayout->addWidget(labeloil2,1,1);
    ngLayout->addWidget(labelcrush1, 2, 0);
    ngLayout->addWidget(labelcrush2,2,1);
    ngLayout->addWidget(labelscratch1, 3, 0);
    ngLayout->addWidget(labelscratch2,3,1);
    ngLayout->addWidget(labelswell1, 4, 0);
    ngLayout->addWidget(labelswell2,4,1);

    ngLayout->addWidget(labellong1, 5, 0);
    ngLayout->addWidget(labellong2,5,1);

    ngLayout->addWidget(labeliron1, 6, 0);
    ngLayout->addWidget(labeliron2,6,1);

    ngLayout->addWidget(labelpush1, 7, 0);
    ngLayout->addWidget(labelpush2,7,1);

    ngLayout->addWidget(labelserial1, 8, 0);
    ngLayout->addWidget(labelserial2,8,1);

    ngLayout->addWidget(labellosta1, 9, 0);
    ngLayout->addWidget(labellosta2,9,1);

    ngLayout->addWidget(labelbatch1, 10, 0);
    ngLayout->addWidget(labelbatch2,10,1);

    ngLayout->setHorizontalSpacing(50);
    ngLayout->setVerticalSpacing(5);
    ngLayout->setContentsMargins(20, 0, 20, 0);
    ngLayout->setAlignment(Qt::AlignCenter);

    ui->btnNgValue->setLayout(ngLayout);
    ui->btnNgValue->setMinimumHeight(220);
    ui->btnNgValue->setStyleSheet("background-color:rgb(68,68,68);");
    ui->btnNgValue->setStyleSheet(QString("border:0px solid write;"));

    QLabel* labeltotal1 = new QLabel();
    labeltotal2 = new QLabel();
    labeltotal1->setStyleSheet(QString("border:0px solid write;"));
    labeltotal2->setStyleSheet(QString("border:0px solid write;"));

    labeltotal1->setText("检查总数");
    labeltotal1->setStyleSheet("color:white");
    labeltotal1->setFixedWidth(100);


    labeltotal2->setText("0");
    labeltotal2->setStyleSheet("color:white");
    labeltotal2->setFixedWidth(90);
    //labelng1->setStyleSheet("background-color:rgb(68,68,68);");
    //labelng2->setStyleSheet("background-color:rgb(68,68,68);");
    labeltotal1->setFont(ft);
    labeltotal2->setFont(ft);
    QHBoxLayout* totalLayout = new QHBoxLayout();
    totalLayout->addSpacing(20);
    totalLayout->addWidget(labeltotal1);
    totalLayout->addSpacing(90);
    totalLayout->addWidget(labeltotal2);
    totalLayout->addStretch();
    ui->btnTotalvalue->setLayout(totalLayout);
    ui->btnTotalvalue->setStyleSheet("background-color:rgb(68,68,68);");
    ui->btnTotalvalue->setStyleSheet(QString("border:0px solid write;"));

    ft.setPointSize(12);
    ui->label_4->setText("AIBox");
    ui->label_4->setFont(ft);
    QPixmap xjp=QPixmap(":/uimain/icon/u20.png");//("icon/a2.png");
    QSize lbsize = ui->label_xj->size();
    xjp.scaled(lbsize,Qt::KeepAspectRatioByExpanding);
    ui->label_xj->setPixmap(xjp);
    ui->label_xj->setScaledContents(true);


    ft.setPointSize(28);
    ui->label_2->setText("100%");
    ui->label_2->setFont(ft);
    ui->label_2->setStyleSheet("color:green");
    ui->label_2->setStyleSheet("border-image:url(:/uimain/icon/h1.png);color:green");
    ui->label_2->setText("0%");
    //i

    ui->stackedWidget->setStyleSheet("QLabel{font:60px;}");
    ui->stackedWidget->setStyleSheet("background-color:rgb(2,6,13);");
    //单独设置指示器大小
    int addWidth = 20;
    int addHeight = 10;
    int rbtnWidth = 15;
    int ckWidth = 13;
    int scrWidth = 12;
    int borderWidth = 3;

    QStringList qss;
    qss << QString("QComboBox::drop-down,QDateEdit::drop-down,QTimeEdit::drop-down,QDateTimeEdit::drop-down{width:%1px;}").arg(addWidth);
    qss << QString("QComboBox::down-arrow,QDateEdit[calendarPopup=\"true\"]::down-arrow,QTimeEdit[calendarPopup=\"true\"]::down-arrow,"
                   "QDateTimeEdit[calendarPopup=\"true\"]::down-arrow{width:%1px;height:%1px;right:2px;}").arg(addHeight);
    qss << QString("QRadioButton::indicator{width:%1px;height:%1px;}").arg(rbtnWidth);
    qss << QString("QCheckBox::indicator,QGroupBox::indicator,QTreeWidget::indicator,QListWidget::indicator{width:%1px;height:%1px;}").arg(ckWidth);
    qss << QString("QScrollBar:horizontal{min-height:%1px;border-radius:%2px;}QScrollBar::handle:horizontal{border-radius:%2px;}"
                   "QScrollBar:vertical{min-width:%1px;border-radius:%2px;}QScrollBar::handle:vertical{border-radius:%2px;}").arg(scrWidth).arg(scrWidth / 2);
    qss << QString("QWidget#widget_top>QToolButton:pressed,QWidget#widget_top>QToolButton:hover,"
                   "QWidget#widget_top>QToolButton:checked,QWidget#widget_top>QLabel:hover{"
                   "border-width:0px 0px %1px 0px;}").arg(borderWidth);
    qss << QString("QWidget#widgetleft>QPushButton:checked,QWidget#widgetleft>QToolButton:checked,"
                   "QWidget#widgetleft>QPushButton:pressed,QWidget#widgetleft>QToolButton:pressed{"
                   "border-width:0px 0px 0px %1px;}").arg(borderWidth);
    this->setStyleSheet(qss.join(""));

    QSize icoSize(32, 32);
    int icoWidth = 85;

    //设置顶部导航按钮
    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }
    frmVideoPanel *Videoview = new frmVideoPanel;


    // Ui::QWidget ui;
    ui->stackedWidget->addWidget(Videoview);
    //ui->stackedWidget->insertWidget(w);
    ui->stackedWidget->setCurrentWidget(Videoview);
    // ui->btnMain->click();
    connect(Videoview->widget, &VideoPanel::senduitotall, this, &frmMain::getuitotall);
    connect(Videoview->widget, &VideoPanel::senduiok, this, &frmMain::getuiok);
    connect(Videoview->widget, &VideoPanel::senduing, this, &frmMain::getuing);
    connect(Videoview->widget, &VideoPanel::sendui, this, &frmMain::getui);
    connect(Videoview->widget, &VideoPanel::sendstatus, this, &frmMain::getstatus);

    connect(this, &frmMain::emit_resendJson, Videoview->widget, &VideoPanel::updateSendJson);

    Videoview->widget->loadHistoryResult();

    ui->label_3->setText(statusStr);
    ui->label_4->setText(enterDay);
}

void frmMain::getstatus(QString str){
    ui->label_3->setText(str);
}

void frmMain::getui()
{
    checkok = _numOK;
    checkng = _numNG;
    checktotall = _numTotal;
    //qDebug("enter get ui\n");
    double d_ok=checkok;
    double d_total=checktotall;
    double tmpvalue = 0;
//    if(findFast==1){
//        ui->label_3->setText("连料停机,请立即重启!");
//        return;
//    }

    if(gpioNG==1){
        ui->label_3->setText("GPIO Error,请立即重启!");
        return;
    }

    if (d_total != 0)
        tmpvalue=d_ok/d_total*100;
    QString  str;
    str = QString::number(tmpvalue,'f',1).append("%");
    //qDebug()<<str;
    ui->label_2->setText(str);
    labeltotal2->setText(QString::number(checktotall));
    labelng2->setText(QString::number(checkng));

    float oilRate=(float)_numOil*100/_numNG;
    labeloil2->setText(QString::number(oilRate,'f',2)+"%");

    float crushRate=(float)_numCrush*100/_numNG;
    labelcrush2->setText(QString::number(crushRate,'f',2)+"%");

    float scratchRate=(float)_numScratch*100/_numNG;
    labelscratch2->setText(QString::number(scratchRate,'f',2)+"%");

    float swellRate=(float)_numSwell*100/_numNG;
    labelswell2->setText(QString::number(swellRate,'f',2)+"%");

    labelpush2->setText(QString::number(

                            pushNumA));
    labellong2->setText(QString::number(longNum));
    labelserial2->setText(QString::number(serialNum));
    labeliron2->setText(QString::number(ironWidth));
    labelbatch2->setText(QString::number(batchNum));
    labellosta2->setText(QString::number(lostA));
    //qDebug("exit get ui\n");
}

void frmMain::getuitotall(int value)
{


}

void frmMain::getuiok(int value)
{


}

void frmMain::getuing(int value)
{

}
void frmMain::buttonClick()
{
    QToolButton *b = (QToolButton *)sender();
    QString name = b->text();

    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setChecked(btn == b);
    }

    if (name == "主界面") {
        frmVideoPanel *w = new frmVideoPanel;
        // Ui::QWidget ui;
        ui->stackedWidget->addWidget(w);
        //ui->stackedWidget->insertWidget(w);
        ui->stackedWidget->setCurrentWidget(w);
        connect(w->widget, &VideoPanel::senduitotall,
                this, &frmMain::getuitotall);
        connect(w->widget, &VideoPanel::senduiok,
                this, &frmMain::getuiok);
        connect(w->widget, &VideoPanel::senduing,
                this, &frmMain::getuing);
        //ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "系统设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "数据查询") {
        frmDbPage *qw = new frmDbPage;
        // Ui::QWidget ui;
        ui->stackedWidget->addWidget(qw);
        //ui->stackedWidget->insertWidget(w);
        ui->stackedWidget->setCurrentWidget(qw);
        // ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "调试帮助") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "用户退出") {
        exit(0);
    }
}

//void frmMain::on_btnMenu_Min_clicked()
//{
//    showMinimized();
//}

//void frmMain::on_btnMenu_Max_clicked()
//{
//    static bool max = false;
//    static QRect location = this->geometry();

//    if (max) {
//        this->setGeometry(location);
//    } else {
//        location = this->geometry();
//        this->setGeometry(QUIHelper::getScreenRect());
//    }

//    this->setProperty("canMove", max);
//    max = !max;
//}

//void frmMain::on_btnMenu_Close_clicked()
//{
//    close();
//}

//zhanglei clear
void frmMain::on_btnsql_clicked()
{
     printf("clear all %d\n",batchNum);
     qDebug("clear all %d\n",batchNum);
     clearAll();
     ui->label_3->setText("清空状态");
     getui();
}

extern  GX_DEV_HANDLE g_hDevice ;
extern GX_STATUS               emStatus;

#define  CAMERA_NUM  1
extern ModbusTcpClient *modbusTcpClient;
extern void* handle[CAMERA_NUM];
int flag=1;
extern  QSqlDatabase dbConn;
extern void getCurrentTimeSec(int channelId, bool is_raw,char * str);
extern void getCurrentTimeDay(char *str);

void frmMain::on_btnstart_clicked()
{
    if(isSuspend==false){
        printf("Enter Suspend");
        isSuspend=true;
        label->setText("开始");
        ui->label_3->setText("当前暂停");
    }else if(isSuspend==true){
        printf("Enter Open");
        isSuspend=false;
        label->setText("暂停");
        ui->label_3->setText("状态正常");
    }
}

void frmMain::on_btnMenu_setting_clicked()
{
    //showMinimized();
    SettingsView *sv = new SettingsView;
    // Ui::QWidget ui;
    //    ui->stackedWidget->addWidget(qw);
    //    //ui->stackedWidget->insertWidget(w);
    //   ui->stackedWidget->setCurrentWidget(qw);
    //connect(sv->Moduleviewwidget, &AlgoSettingWidget::emit_sendJson, this, &frmMain::getJsonId);
    sv->show();

}

void frmMain::getJsonId(int id)
{
    emit emit_resendJson(id);
}

void frmMain::on_btnMenu_refresh_clicked()
{
    frmDbPage *qw = new frmDbPage;

    qw->show();
}



void frmMain::on_btnMenu_refresh_pressed()
{

}

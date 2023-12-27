#include "gxview.h"
#include "ui_gxview.h"

Gxview::Gxview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Gxview)
{

    ui->setupUi(this);

    QLabel* label = new QLabel();
    QLabel* label2 = new QLabel();
    label2->setFixedSize(15,15);
    label->setFixedSize(50,20);
    label2->setStyleSheet(QString("border:0px solid write;"));
    label->setStyleSheet(QString("border:0px solid write;"));
    QPixmap pix=QPixmap(":/uimain/icon/u17.png");
    QSize size = label2->size();
    pix.scaled(size,Qt::KeepAspectRatioByExpanding);
    label2->setPixmap(pix);
    label2->setScaledContents(true);
    label->setText("开始运行");
    label->setStyleSheet("color:white");
    label->setFixedWidth(50);
    label->setScaledContents(true);
    QHBoxLayout* myLayout = new QHBoxLayout();
    myLayout->addSpacing(1);
    myLayout->addWidget(label2);
    myLayout->addSpacing(1);
    myLayout->addWidget(label);
    myLayout->addStretch();
    ui->btngxopen->setStyleSheet("background-color:rgb(28,189,127);");
    ui->btngxopen->setLayout(myLayout);

    QLabel* labelsql1 = new QLabel();
    QLabel* labelsql2 = new QLabel();
    labelsql2->setFixedSize(15,15);
    labelsql1->setFixedSize(50,20);
    labelsql2->setStyleSheet(QString("border:0px solid write;"));
    labelsql1->setStyleSheet(QString("border:0px solid write;"));
    QPixmap pixsql=QPixmap(":/uimain/icon/u502.png");
    QSize sqlsize = labelsql2->size();
    pixsql.scaled(sqlsize,Qt::KeepAspectRatioByExpanding);
    labelsql2->setPixmap(pixsql);
    labelsql2->setScaledContents(true);
    labelsql1->setText("数据查询");
    labelsql1->setStyleSheet("color:white");
    labelsql1->setFixedWidth(50);
    labelsql1->setScaledContents(true);
    QHBoxLayout* sqlLayout = new QHBoxLayout();
    sqlLayout->addSpacing(1);
    sqlLayout->addWidget(labelsql2);
    sqlLayout->addSpacing(1);
    sqlLayout->addWidget(labelsql1);
    sqlLayout->addStretch();
    ui->btngxclose->setStyleSheet("background-color:rgb(22,155,213);");
    ui->btngxclose->setLayout(sqlLayout);
}

Gxview::~Gxview()
{

    delete ui;
}




void Gxview::module_on_pushButton_clicked()
{
    //    QString name = ui->lineEdit->text();
    //    if (!name.isEmpty()) {
    //        QString str = QString("id = '%1'").arg(name);
    //        model->setFilter(str);
    //    } else {
    //        //清空过滤设置，并查询
    //        opeStr = "insert into line1data (id, ret, date,location) values (3, 'ok', 20221017,'D:/data/3.bmp');";

    //         inserttoDb(query, opeStr);
    //      //  model->setFilter("");
    //      //  model->select();
    //   }
}

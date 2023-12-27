#pragma execution_character_set("utf-8")

#include "frmdbpage.h"
#include "ui_frmdbpage.h"
#include "dbpage.h"
#include "iconhelper.h"
#include "quihelper.h"
#include <iostream>

frmDbPage::frmDbPage(QWidget *parent) : QWidget(parent,Qt::WindowStaysOnTopHint), ui(new Ui::frmDbPage)
{
    ui->setupUi(this);
    this->initForm();
    this->setWindowFlags( Qt::WindowStaysOnTopHint|Qt::Tool);
    dateview=ui->comboBox;
    //  QUIHelper::setFramelessForm(this);
    on_btnSelect_clicked();
    setGeometry(300,300,800,600 );
}

frmDbPage::~frmDbPage()
{
    delete ui;
}

void frmDbPage::initForm()
{
    columnNames.clear();
    columnWidths.clear();

    // tableName = "new";
    tableName = "workpieces";
    countName = "date";
    columnNames.append("时间");
    columnNames.append("批次");
    columnNames.append("OK数");
    columnNames.append("NG数");
    columnNames.append("总数");

    columnNames.append("油污");
    columnNames.append("压伤");
    columnNames.append("翘边");
    columnNames.append("划痕");
    columnNames.append("相机到位");

    columnNames.append("串口到位");
    columnNames.append("A面错位");
    columnNames.append("图片地址");



    int colW = 50;
    columnWidths.append(colW*4);
    columnWidths.append(colW*4);
    columnWidths.append(colW);
    columnWidths.append(colW);
    columnWidths.append(colW);

    columnWidths.append(colW);
    columnWidths.append(colW);
    columnWidths.append(colW);
    columnWidths.append(colW);
    columnWidths.append(80);

    columnWidths.append(80);
    columnWidths.append(80);
    columnWidths.append(180*2);

    //设置需要显示数据的表格和翻页的按钮
    dbPage = new DbPage(this);
    //设置所有列居中显示
    dbPage->setAllCenter(true);
    dbPage->setControl(ui->tableMain, ui->labPageTotal, ui->labPageCurrent, ui->labRecordsTotal, ui->labRecordsPerpage,
                       ui->labSelectTime, 0, ui->btnFirst, ui->btnPreVious, ui->btnNext, ui->btnLast, countName);
    ui->tableMain->horizontalHeader()->setStretchLastSection(true);
    ui->tableMain->verticalHeader()->setDefaultSectionSize(25);
}
extern QSqlQuery query;
extern  QSqlDatabase dbConn;

void frmDbPage::on_btnSelect_clicked()
{
    //绑定数据到表格
    QString sql = "where 1=1";
    printf("start\n");

    if(dateview->start.isEmpty() == true)
    {
        dbPage->setTableName(tableName);
    }else{
        sql = QString("where date between '%1' and '%2'").arg(dateview->start).arg(dateview->end);
        dbPage->setTableName(tableName);
    }

    dbPage->setOrderSql(QString("%1 %2").arg(countName).arg("desc"));
    dbPage->setWhereSql(sql);
    dbPage->setRecordsPerpage(20);
    dbPage->setColumnNames(columnNames);
    dbPage->setColumnWidths(columnWidths);
    dbPage->select();
}

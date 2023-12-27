#include "ftpview.h"
#include "ui_ftpview.h"

Ftpview::Ftpview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Ftpview)
{

    ui->setupUi(this);

}

Ftpview::~Ftpview()
{

    delete ui;
}




void Ftpview::ftp_on_pushButton_clicked()
{
 //   QString name = ui->lineEdit->text();
//  //  if (!name.isEmpty()) {
//     //   QString str = QString("id = '%1'").arg(name);
//     //   model->setFilter(str);
//    } else {
//        //清空过滤设置，并查询
//        opeStr = "insert into line1data (id, ret, date,location) values (3, 'ok', 20221017,'D:/data/3.bmp');";

//         inserttoDb(query, opeStr);
//      //  model->setFilter("");
//      //  model->select();
//    }
}

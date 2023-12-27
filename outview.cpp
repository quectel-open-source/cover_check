#include "outview.h"
#include "ui_outview.h"

Outview::Outview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Outview)
{

    ui->setupUi(this);

}

Outview::~Outview()
{

    delete ui;
}




void Outview::out_on_pushButton_clicked()
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

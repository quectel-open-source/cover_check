#pragma execution_character_set("utf-8")
#include "frmvideopanel.h"
#include "ui_frmvideopanel.h"
#include "frmmain.h"

#include "ModbusTcpClient.h"

extern ModbusTcpClient *modbusTcpClient;
frmVideoPanel::frmVideoPanel(QWidget *parent) : QWidget(parent), ui(new Ui::frmVideoPanel)
{
    ui->setupUi(this);
    widget=ui->widget;

    //modbusTcpClient= new ModbusTcpClient(widget);

}

frmVideoPanel::~frmVideoPanel()
{
    delete ui;
}

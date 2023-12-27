#include <QWidget>
#include <QDebug>
#include <QMessageBox>
#include "QuickSetThread.h"

namespace Ui {
class Quickview;
}

class Quickview : public QWidget
{
    Q_OBJECT

public:
    explicit Quickview(QWidget *parent = 0);
    ~Quickview();

public slots:
    void on_pushButton_clicked();
    void showMsg(QString str);
private:
    int JsonIORead(QString path);
    int JsonIOWrite(QString path);
    Ui::Quickview *ui;
    QuickSetThread* qs;
    QString opeStr;

};

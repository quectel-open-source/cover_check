#include <QWidget>
#include <QDebug>
#include <QMessageBox>


namespace Ui {
class Transferview;
}

class Transferview : public QWidget
{
    Q_OBJECT

public:
    explicit Transferview(QWidget *parent = 0);
    ~Transferview();


private slots:
//    void on_addButton_clicked();

//    void on_delButton_clicked();

//    void on_confirmButton_clicked();

//    void on_cancelButton_clicked();

    void transfer_on_pushButton_clicked();

private:
    Ui::Transferview *ui;

    QString opeStr;

};

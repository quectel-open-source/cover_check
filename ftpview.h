#include <QWidget>
#include <QDebug>
#include <QMessageBox>


namespace Ui {
class Ftpview;
}

class Ftpview : public QWidget
{
    Q_OBJECT

public:
    explicit Ftpview(QWidget *parent = 0);
    ~Ftpview();


private slots:
//    void on_addButton_clicked();

//    void on_delButton_clicked();

//    void on_confirmButton_clicked();

//    void on_cancelButton_clicked();

    void ftp_on_pushButton_clicked();

private:
    Ui::Ftpview *ui;

    QString opeStr;

};

#include <QWidget>
#include <QDebug>
#include <QMessageBox>


namespace Ui {
class Outview;
}

class Outview : public QWidget
{
    Q_OBJECT

public:
    explicit Outview(QWidget *parent = 0);
    ~Outview();


private slots:
//    void on_addButton_clicked();

//    void on_delButton_clicked();

//    void on_confirmButton_clicked();

//    void on_cancelButton_clicked();

    void out_on_pushButton_clicked();

private:
    Ui::Outview *ui;

    QString opeStr;

};

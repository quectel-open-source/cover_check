#include <QWidget>
#include <QDebug>
#include <QMessageBox>


namespace Ui {
class Gxview;
}

class Gxview : public QWidget
{
    Q_OBJECT

public:
    explicit Gxview(QWidget *parent = 0);
    ~Gxview();


private slots:
//    void on_addButton_clicked();

//    void on_delButton_clicked();

//    void on_confirmButton_clicked();

//    void on_cancelButton_clicked();

    void module_on_pushButton_clicked();

private:
    Ui::Gxview *ui;

    QString opeStr;

};

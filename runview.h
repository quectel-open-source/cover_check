#include <QWidget>
#include <QDebug>
#include <QMessageBox>


namespace Ui {
class Runview;
}

class Runview : public QWidget
{
    Q_OBJECT

public:
    explicit Runview(QWidget *parent = 0);
    ~Runview();

private slots:

    void on_pushButton_clicked();
    int JsonIORead(QString path);
    int JsonIOWrite(QString path);
private:
    Ui::Runview *ui;

    QString opeStr;

};

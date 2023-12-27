#include <QWidget>
#include <QDebug>
#include <QMessageBox>
#include "form/common.h"

namespace Ui {
class Moduleview;
}

class Moduleview : public QWidget
{
    Q_OBJECT

public:
    explicit Moduleview(QWidget *parent = 0);
    ~Moduleview();


private slots:
//    void on_addButton_clicked();

//    void on_delButton_clicked();

//    void on_confirmButton_clicked();

//    void on_cancelButton_clicked();

    void module_on_pushButton_clicked();

private:
    Ui::Moduleview *ui;

    QString opeStr;

};

typedef struct defectParam_suf{
    int pre_th;
    int bruise[4];
    int dirt[4];
    int coarse[4];
    int lidless[4];
    int lidless1[4];
    int uneven[2];
    bool isSaved;
}defectParam_suf;

typedef struct defectParam_pit{
    int pre_th;
    int pit[4];
    bool isSaved;
}defectParam_pit;

typedef struct defectParam_side{
    int pre_th;
    int bruise[4];
    int scratch[4];
    int dirt[4];
    int coarse[4];
    int coarse1[4];
    bool isSaved;
}defectParam_side;

class AlgoSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlgoSettingWidget(QWidget *parent = 0);
    ~AlgoSettingWidget();

    QLineEdit* lineEdit_zhouCheng;

    QLineEdit* lineEdit_conf;
    QLineEdit* lineEdit_iou;
    QLineEdit* lineEdit_score;

    QLineEdit* lineEditUsb1;
    QLineEdit* lineEditInnerFreq1;
    QLineEdit* lineEditOutputNum1;

    QLineEdit* lineEditUsb2;
    QLineEdit* lineEditInnerFreq2;
    QLineEdit* lineEditOutputNum2;

    QWidget* wDefect_1;
    QWidget* wDefect_2;
    QWidget* wDefect_3;

    QWidget* wParam123;

    QToolButton* buttonYes;

    QStringList _defectNameList;
    QList<QCheckBox*> _checkBoxList;
    QList<QJsonObject> _objs;
    QStringList _jsonPath;

    QLabel* labelStatus;

    double _confThresh;
    double _iouThresh;
    double _scoreThresh;
private:
    void init_UI();

private slots:
    void on_toolButton_clicked();
    int lightSerial1_readyRead();
    int lightSerial2_readyRead();

private:
    int JsonIORead(QString path);
    int JsonIOWrite(QString path);
    QString opeStr;

};

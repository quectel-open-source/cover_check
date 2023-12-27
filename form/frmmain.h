#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QDialog>
#include <QWidget>
#include <QMap>
#include <QTcpSocket>
#include <QElapsedTimer>
#include "MvCamera.h"
#include "qsqltablemodel.h"


class QLabel;
class QToolButton;
class QStackedWidget;
class QString;

namespace Ui {
class frmMain;
}

class frmMain : public QDialog
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();
    QLabel* label;

    QLabel* labeltotal2;
    QLabel* labelng2;
    QLabel* labelserial2;
    QLabel* labeliron2;
    QLabel* labellong2;
    QLabel* labellosta2;

    QLabel* labelbatch2;

    QLabel* labelpush2;


    //QLabel* labelmem2;
    QLabel* labeloil2;
    QLabel* labelcrush2;
    QLabel* labelscratch2;
    QLabel* labelswell2;

    int checktotall;
    int checkng;
    int checkok;


protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::frmMain *ui;
    QToolButton * _activeButton;
    int m_buff_size;
    QElapsedTimer elaptimer;
    QByteArray m_buffer;
    int pkgCount;
    QSqlTableModel *model;

private slots:
    void initForm();
    void buttonClick();

private slots:
    //void on_btnMenu_Min_clicked();
    //void on_btnMenu_Max_clicked();
    void on_btnstart_clicked();
    void on_btnsql_clicked();
    //void on_btnMenu_Close_clicked();
    void on_btnMenu_setting_clicked();
    void on_btnMenu_refresh_clicked();
    void getuitotall(int value);
    void getuing(int value);
    void getuiok(int value);
    void getui();
    void getstatus(QString str);

    void on_btnMenu_refresh_pressed();

    void getJsonId(int id);

signals:
    void emit_resendJson(int jsonId);

Q_SIGNALS:
    //全屏切换信号
    void qfullScreen(bool full);
};

#endif // UIDEMO01_H

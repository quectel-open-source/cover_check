#ifndef DATE_TIME_EDIT_H
#define DATE_TIME_EDIT_H

#include <QComboBox>

class DateTimeEdit : public QComboBox
{
    Q_OBJECT
public:
    explicit DateTimeEdit(QWidget *parent = 0);
    QString  start;
    QString  end;

public Q_SLOTS:
    void gettext(QString  text);
    void getstart(QString  text);
    void getend(QString  text);

private:
    void initPage();



};

#endif // DATE_TIME_EDIT_H

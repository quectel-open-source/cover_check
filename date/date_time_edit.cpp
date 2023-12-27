#include "date_time_edit.h"
#include "date_time_range_widget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>
DateTimeEdit::DateTimeEdit(QWidget *parent) : QComboBox(parent)
{
    initPage();
}

void DateTimeEdit::initPage()
{
    auto table = new QTableWidget;
    table->setMinimumHeight(520);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setColumnCount(1);
    table->setRowCount(1);

    auto cell = new DateTimeRangeWidget;

    table->setCellWidget(0, 0, cell);

    this->setModel(table->model());
    this->setView(table);

    connect(cell, &DateTimeRangeWidget::signal_dateTime, this, [this](const QString& text) {
        this->setEditText(text);
        this->hidePopup();

    });
    connect(cell, &DateTimeRangeWidget::signal_dateTime, this, &DateTimeEdit::gettext);
    connect(cell, &DateTimeRangeWidget::signal_start, this, &DateTimeEdit::getstart);
    connect(cell, &DateTimeRangeWidget::signal_end, this, &DateTimeEdit::getend);
    connect(cell, &DateTimeRangeWidget::signal_cancel, this, [=] {
        hidePopup();
    });
}
void DateTimeEdit::gettext(QString  text)
{
    qDebug() << text;


}
void DateTimeEdit::getstart(QString  text)
{
    qDebug() << text;
    start = text;

}
void DateTimeEdit::getend(QString  text)
{
    qDebug() << text;
    end = text;

}

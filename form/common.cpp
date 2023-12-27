#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QObject>

using namespace std;


QLabel* newLabel(QString text, int w, int h)
{
    QLabel* label = new QLabel;
    label->setText(text);
    if (w > 0)
        label->setFixedWidth(w);
    if (h > 0)
        label->setFixedHeight(h);

    return label;
}

QLineEdit* newLineEdit(QString text, int w, int h, QList<int> validator, QString qssStr)
{
    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setText(text);
    if (w > 0)
        lineEdit->setFixedWidth(w);
    if (h > 0)
        lineEdit->setFixedHeight(h);
//    if (validator[1] > 0)
//        lineEdit->setValidator((QValidator)QIntValidator(validator[0], validator[1]));
    if (qssStr.compare("") != 0)
        lineEdit->setStyleSheet(qssStr);

    return lineEdit;
}

QCheckBox* newCheckBox(QString text, int w, int h, bool isChecked) //, QWidget* receiver, void* slot)
{
    QCheckBox* checkBox = new QCheckBox;
    checkBox->setText(text);
    if (w > 0)
        checkBox->setFixedWidth(w);
    if (h > 0)
        checkBox->setFixedHeight(h);
    checkBox->setChecked(isChecked);

//    if (receiver != NULL)
//        QObject::connect(checkBox, &QCheckBox::clicked, receiver, slot);

    return checkBox;
}

QToolButton* newToolButton(QString text, int w, int h, QString iconPath, bool checkable, bool isChecked, Qt::ToolButtonStyle style, QString qssStr)
{
    QToolButton* button = new QToolButton;
    button->setText(text);
    if (w > 0)
        button->setFixedWidth(w);
    if (h > 0)
        button->setFixedHeight(h);

    if (iconPath.compare("") != 0){
        button->setIcon(QIcon(iconPath));
    }

    if (checkable){
        button->setCheckable(checkable);
        button->setChecked(isChecked);
    }
    else
        button->setCheckable(checkable);


    button->setToolButtonStyle(style);
    if (qssStr.compare("") != 0)
        button->setStyleSheet(qssStr);

    return button;
}


QFrame* newSplit(int w, int h)
{
    QFrame* frame = new QFrame;
    frame->setFrameShape(QFrame::VLine);
    frame->setFrameShadow(QFrame::Sunken);
    frame->setFixedSize(w, h);
    return frame;
}

void newHBoxLayout(QWidget* parent, QList<QWidget*> widgets, int splitW, int space, QList<int> margins, Qt::Alignment align)
{
    QHBoxLayout* layout = new QHBoxLayout(parent);
    foreach(QWidget* w, widgets)
    {
        layout->addWidget(w);
    }
    layout->setSpacing(space);
    layout->setContentsMargins(margins[0], margins[1], margins[2], margins[3]);
    layout->setAlignment(align);
}

void newVBoxLayout(QWidget* parent, QList<QWidget*> widgets, int splitW, int space, QList<int> margins, Qt::Alignment align)
{
    QVBoxLayout* layout = new QVBoxLayout(parent);
    foreach(QWidget* w, widgets)
    {
        layout->addWidget(w);
    }
    layout->setSpacing(space);
    layout->setContentsMargins(margins[0], margins[1], margins[2], margins[3]);
    layout->setAlignment(align);
}

QWidget* newWidgetLine(int w, int h, QString qssStr)
{
    QWidget* wLine = new QWidget;
    wLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (w > 0)
        wLine->setFixedWidth(w);
    if (h > 0)
        wLine->setFixedHeight(h);
    wLine->setStyleSheet(qssStr);

    return wLine;
}

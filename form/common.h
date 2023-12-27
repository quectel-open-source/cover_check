#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <QtCore>
#include <Qt>
#include <QtGui>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>


QLabel* newLabel(QString text="", int w=0, int h=0);
QLineEdit* newLineEdit(QString text="", int w=0, int h=0, QList<int> validator={0, 0}, QString qssStr="border:2px solid rgb(119,183,244); background-color:transparent;");
QCheckBox* newCheckBox(QString text="", int w=0, int h=0, bool isChecked=false); //, QWidget* receiver=NULL, void* slot=NULL);
QToolButton* newToolButton(QString text="", int w=0, int h=0, QString iconPath="", bool checkable=true, bool isChecked=false, Qt::ToolButtonStyle style=Qt::ToolButtonTextOnly,
                           QString qssStr="QToolButton{border:2px solid rgb(119,183,244);}, QToolButton:hover{background-color:rgb(119,183,244);}");

QFrame* newSplit(int w, int h);
void newHBoxLayout(QWidget* parent, QList<QWidget*> widgets, int splitW=0, int space=0, QList<int> margins={0,0,0,0}, Qt::Alignment align=Qt::AlignCenter);
void newVBoxLayout(QWidget* parent, QList<QWidget*> widgets, int splitW=0, int space=0, QList<int> margins={0,0,0,0}, Qt::Alignment align=Qt::AlignCenter);

QWidget* newWidgetLine(int w=0, int h=0, QString qssStr="");

#endif // COMMON_H

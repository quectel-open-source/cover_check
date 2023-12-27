#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
#include <QMap>
#include "moduleview.h"
#include "ui_moduleview.h"
#include "ftpview.h"
#include "ui_ftpview.h"
#include "frmvideopanel.h"

#include "runview.h"
#include "ui_runview.h"

#include "quickview.h"
#include "ui_quickview.h"

#include "algoview.h"
#include "ui_algoview.h"

#include "transferview.h"
#include "ui_transferview.h"
#include "outview.h"
#include "ui_outview.h"
#include "gxview.h"
#include "ui_gxview.h"


class QLabel;
class QToolButton;
class QStackedWidget;
class QString;

class SettingsView : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsView(QWidget *parent = 0);
    ~SettingsView();

    void push(QWidget * page);
    void pop();

public slots:
    void changeCenterWidget(bool);

private:
    QToolButton * createSidebarButton(const QString& iconPath, const QString& title);
    QStackedWidget * _stackedWidget;
    QToolButton * _activeButton;

    Quickview *quickview;
    Runview *runview;
    Gxview *Gxviewwidget;
    Ftpview *Ftpviewwidget ;
    Transferview *Transferviewwidget ;
    Outview *Outviewwidget ;
public:
    Algoview *algoview;
    AlgoSettingWidget *Moduleviewwidget ;

};

#endif /* SETTINGSVIEW_H */

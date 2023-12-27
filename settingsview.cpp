#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QToolButton>
#include <QDebug>
#include <QStackedWidget>

#include "settingsview.h"
#include "borderlayout.h"


/*------------------------------------------------------------------------------
 * CTOR / DTOR
 *----------------------------------------------------------------------------*/
/**
 * @brief Create a instance of the settings widget
 *
 * @param parent
 */
SettingsView::SettingsView(QWidget *parent) :
    QWidget(parent),
    _stackedWidget(nullptr),
    _activeButton(nullptr)
{
    /* Create a layout for the sidebar */
    QWidget * sidebar = new QWidget();
    QVBoxLayout * sidebarLayout = new QVBoxLayout();
    _activeButton = createSidebarButton(":/icons/assets/zoom.svg", tr("快速设置"));
    _activeButton->setChecked(true);
    sidebarLayout->addWidget(_activeButton);
    this->setWindowFlags( Qt::WindowStaysOnTopHint|Qt::Tool);

    sidebarLayout->addWidget(createSidebarButton(":/icons/assets/flower.svg", tr("微调设置") ));
    sidebarLayout->addWidget(createSidebarButton(":/icons/assets/flower.svg", tr("光源设置") ));
    sidebarLayout->addWidget(createSidebarButton(":/icons/assets/flower.svg", tr("算法设置") ));

    sidebarLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    sidebarLayout->setSpacing(0);
    sidebarLayout->setMargin(0);
    /* Add the sidebar layout to the sidebar widget container */
    sidebar->setLayout(sidebarLayout);
    sidebar->setObjectName("sidebar");
    sidebar->setMinimumHeight((sidebarLayout->count()+2) * 76);

    /* Create the stacked widget */
    _stackedWidget = new QStackedWidget;

    /* Create the board layout */
    BorderLayout *layout = new BorderLayout();
    layout->addWidget(_stackedWidget, BorderLayout::Center);
    layout->addWidget(sidebar, BorderLayout::West);
    setLayout(layout);
    layout->setSpacing(0);
    QPalette pal;
    pal.setColor(QPalette::Background,QColor(14,34,59));
    _stackedWidget->setAutoFillBackground(true);
    _stackedWidget->setPalette(pal);

    pal.setColor(QPalette::Background,QColor(30,41,54));
    sidebar->setAutoFillBackground(true);
    sidebar->setPalette(pal);

    setWindowTitle(tr("快速设置"));
    setGeometry(400,400,800, sidebar->minimumHeight());

    /* Create the first view*/

    quickview = new Quickview;
    QWidget *widget = new QWidget;
    Ui::Quickview ui;
    ui.setupUi(widget);
    push(quickview);
}

/**
 * @brief Free allocated memory
 */
SettingsView::~SettingsView()
{
    delete _stackedWidget; _stackedWidget = nullptr;
}

/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
/**
 * @brief Slot to change the center widget
 *
 * @param event True if touched and false if released.
 */
void SettingsView::changeCenterWidget(bool event)
{
    Q_UNUSED(event);
    QString sender = QObject::sender()->objectName();

    qDebug() << "changeCenterWidget  =============";

    if(_activeButton != nullptr) {
        _activeButton->setChecked(false);
        _activeButton->setStyleSheet("background-color:rgb(30,41,54);");
    }

    _activeButton = static_cast<QToolButton*>(QObject::sender());
    _activeButton->setChecked(true);
    _activeButton->setStyleSheet("background-color:rgb(22,153,211);");

    /* Remove all views from the stack if something is available */
    while(_stackedWidget->count() > 0)
    {
        pop();
    }

    if(sender.compare("快速设置") == 0) {
            quickview = new Quickview;
            QWidget *widget = new QWidget;
            Ui::Runview ui;
            ui.setupUi(widget);
            push(quickview);
     }else if(sender.compare("微调设置")==0) {
            runview = new Runview;
            QWidget *widget = new QWidget;
            Ui::Runview ui;
            ui.setupUi(widget);
            push(runview);
     }else if(sender.compare("光源设置") == 0) {

        Moduleviewwidget = new AlgoSettingWidget;
        QWidget *widget = new QWidget;
        Ui::Moduleview ui;
        ui.setupUi(widget);
        push(Moduleviewwidget);
     }else if(sender.compare("算法设置") == 0) {
        algoview = new Algoview;
        QWidget *widget = new QWidget;
        Ui::Moduleview ui;
        ui.setupUi(widget);
        push(algoview);
     }

}

/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/

/**
 * @brief Create a button for the sidebar
 *
 * @param iconPath Path to the icon
 * @param title Tile to display under the icon
 *
 * @return A new instance of a button for the sidebar
 */
QToolButton * SettingsView::createSidebarButton(const QString& iconPath, const QString& title)
{
    QIcon icon(iconPath);

    QToolButton * btn = new QToolButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(42, 42));
    btn->setText(title);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setFixedSize(160, 160);
    btn->setObjectName(title);
    btn->setCheckable(true);
    btn->setStyleSheet("background-color:rgb(30,41,54);QPushButton{text-align:left};");
    QObject::connect(btn, SIGNAL(clicked(bool)),
                     this, SLOT(changeCenterWidget(bool)));

    return btn;
}

/**
 * @brief Push the widget on the stack and set it active
 *
 * @param page Widget to push
 */
void SettingsView::push(QWidget *page)
{
    _stackedWidget->addWidget(page);
    _stackedWidget->setCurrentWidget(page);
    qDebug() << "push =="<<_stackedWidget->currentIndex();
    qDebug() << "count =="<<_stackedWidget->count();
    qDebug() << "currentIndex =="<<_stackedWidget->currentIndex();
}

/**
 * @brief Remove the current widget from the stack and switch to the previous
 */
void SettingsView::pop()
{
    QWidget * currentWidget = _stackedWidget->currentWidget();
    _stackedWidget->removeWidget(currentWidget);

    delete currentWidget; currentWidget = nullptr;
}

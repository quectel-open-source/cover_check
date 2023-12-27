#ifndef VIDEOBOX_H
#define VIDEOBOX_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include "MvCamera.h"
class QMenu;
class QWidget;
class QGridLayout;

#ifdef quc
class Q_DECL_EXPORT VideoBox : public QObject
#else
class VideoBox : public QObject
#endif

{
    Q_OBJECT

public:
    explicit VideoBox(QObject *parent = 0);

private:
    //表格布局存放通道
    QGridLayout *gridLayout;
    //视频控件集合
    QWidgetList widgets;

    //通道数量
    int videoCount;
    //当前画面类型
    QString videoType;

    //主菜单子菜单文字标识
    QString menuFlag;
    QString actionFlag;

    //布局方案标识集合
    QMap<int, QStringList> types;
    void addMenu(QMenu *menu, int type);

    void *m_hWnd;

    MV_CC_DEVICE_INFO_LIST  m_stDevList;
    CMvCamera*              m_pcMyCamera;
    bool                    m_bGrabbing;   // 是否开始抓图

public Q_SLOTS:
    //设置当前画面类型
    void setVideoType(const QString &videoType);
    //设置表格布局
    void setLayout(QGridLayout *gridLayout);
    //设置视频控件集合
    void setWidgets(QWidgetList widgets);

    //设置主菜单子菜单文字标识
    void setMenuFlag(const QString &menuFlag);
    void setActionFlag(const QString &actionFlag);

    //设置子菜单类型集合
    void setTypes(const QMap<int, QStringList> &types);
    //初始化菜单
    void initMenu(QMenu *menu, const QList<bool> &enable);

    //通用设置函数
    void show_video(int type, int index);
    //菜单切换布局槽函数
    void show_video();

    //显示和隐藏所有通道
    void show_video_all();
    void hide_video_all();

    //常规及异形通道布局
    void change_video_normal(int index, int flag);
    void change_video_custom(int index, int type);

    //异形布局
    void change_video_6(const QList<int> &indexs);
    void change_video_8(const QList<int> &indexs);
    void change_video_13(const QList<int> &indexs);

    //具体通道切换函数
    void change_video_1(int index);
    void change_video_4(int index);
    void change_video_6(int index);
    void change_video_8(int index);
    void change_video_9(int index);
    void change_video_13(int index);
    void change_video_16(int index);
    void change_video_25(int index);
    void change_video_36(int index);
    void change_video_64(int index);

Q_SIGNALS:
    //画面布局切换信号
    void changeVideo(int type, const QString &videoType, bool videoMax);
};

#endif // VIDEOBOX_H

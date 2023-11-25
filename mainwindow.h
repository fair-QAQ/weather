#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QLabel>
#include<QPoint>
#include<QDebug>

#include"ewqatherdata.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void getWeatherInfo(QString cityCode);
    void weatherType();
    void updateUI();

    //重写父类的eventFilter方法
    bool eventFilter(QObject *watched, QEvent *ev);

    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();



private slots:
    void on_btnSearch_clicked();

    void on_leCity_returnPressed();

private:
    //处理天气数据
    void onReplied(QNetworkReply* reply);
    void parseJson(QByteArray &bytearray);


private:
    Ui::MainWindow *ui;
    QNetworkAccessManager * mNetworkAccessManager;
    Today mToday; //当天的天气数据
    Day mDay[6];  //未来六天的天气数据






    //控件数组,用于更新UI
    //星期和日期
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;
    //天气和天气图标
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIconList;
    //天气污染指数
    QList<QLabel*> mAqiList;
    //风力和风向
    QList<QLabel*> mFxList;
    QList<QLabel*> mFlList;

    QMap<QString,QString> mTypeMap;

};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include"weathertool.h"
#include<QPainter>
#define POINT_RADIUS 2  //画笔画圆半径
#define HHH  3  //点到y轴
#define  X_pianyi  10
#define  Y_pianyi  10


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mNetworkAccessManager = new QNetworkAccessManager(this);
    connect(mNetworkAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);
     weatherType();
    getWeatherInfo("武汉");
    //安装绘图事件管理器
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onReplied(QNetworkReply *reply)
{
    int status_code =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

     if(status_code != 200)
    {
        qDebug()<<reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }
    else
    {
        QByteArray bytearray =  reply->readAll();
        //qDebug()<<"read all:"<<bytearray.data();
        parseJson(bytearray);
    }

    reply->deleteLater(); //释放内存

}
void MainWindow::getWeatherInfo(QString cityname)
{
    QString cityCode =  weatherTool::GetCity(cityname);
    QUrl url("http://t.weather.sojson.com/api/weather/city/"+cityCode);
    mNetworkAccessManager->get(QNetworkRequest(url));
}




void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray,&err);

    if(err.error != QJsonParseError::NoError)
    {
    qDebug()<<"返回的数据不是json格式！";
         return;
    }
    // 1. 解析日期和城市
    QJsonObject obj = doc.object();
    mToday.date =obj.value("date").toString();
    mToday.city = obj.value("cityInfo").toObject().value("city").toString();
    //qDebug()<<"today is date =" <<mToday.date;
    //qDebug()<<"city"<<mToday.city;

     //2,解析yesterday
    QJsonObject objdata = obj.value("data").toObject();
    QJsonObject obj_yesterday = objdata.value("yesterday").toObject();
    mDay[0].week  = obj_yesterday.value("week").toString();
    mDay[0].date  = obj_yesterday.value("ymd").toString();
    mDay[0].type = obj_yesterday.value("type").toString();


    QString s;  //高温 与 低温
    s = obj_yesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].high = s.toInt();

    s = obj_yesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].low = s.toInt();

    //风向与风力
    mDay[0].fx = obj_yesterday.value("fx").toString();
    mDay[0].fl = obj_yesterday.value("fl").toString();

    mDay[0].aqi = obj_yesterday.value("aqi").toDouble();

    // 3. 解析forecast中5天的数据
        QJsonArray forecastArr = objdata.value("forecast").toArray();

        //mDay[0]表示昨天的数据,已经赋值了,这里循环给未来五天赋值 i+1
        for(int i=0; i<5; i++)
        {
           QJsonObject objforecast = forecastArr[i].toObject();
           mDay[i+1].week = objforecast.value("week").toString();
           mDay[i+1].date = objforecast.value("ymd").toString();

           mDay[i+1].type = objforecast.value("type").toString();

           QString s;
           s = objforecast.value("high").toString().split(" ").at(1);
           s = s.left(s.length()-1);
           mDay[i+1].high = s.toInt();

           s = objforecast.value("low").toString().split(" ").at(1);
           s = s.left(s.length()-1);
           mDay[i+1].low = s.toInt();

           mDay[i+1].fx = objforecast.value("fx").toString();
           mDay[i+1].fl = objforecast.value("fl").toString();

           mDay[i+1].aqi = objforecast.value("aqi").toDouble();
        }
        // 4. 解析今天的数据
        mToday.ganmao = objdata.value("ganmao").toString();

        mToday.wendu = objdata.value("wendu").toString();
        mToday.shidu = objdata.value("shidu").toString();
        mToday.pm25 = objdata.value("pm25").toDouble();
        mToday.quality = objdata.value("quality").toString();

        //qDebug()<<"///////"<<mToday.ganmao;

        // 5. forecast中的第一个数组元素也是今天的数据
        mToday.type = mDay[1].type;
        mToday.fx = mDay[1].fx;
        mToday.fl = mDay[1].fl;
        mToday.high = mDay[1].high;
        mToday.low = mDay[1].low;
        updateUI();
        ui->lblHighCurve->update();
        ui->lblLowCurve->update();
}


void MainWindow::updateUI()
{
    // 日期和城市
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")+" "+mDay[1].week);
    ui->lblCity->setText(mToday.city);

    // 更新今天的数据
    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblTemp->setText(mToday.wendu+"°");
    //ui->lblType->setText(mToday.type);
    ui->lblLowHigh->setText(QString::number(mToday.low) + "~" +QString::number(mToday.high) + "°C");

    ui->lblGanMao->setText("感冒指数:" + mToday.ganmao);
    ui->lblWindFx->setText(mToday.fx);
    ui->lblWindFl->setText(mToday.fl);

    ui->lblPM25->setText(QString::number(mToday.pm25));

    ui->lblShiDu->setText(mToday.shidu);
    ui->lblQuality->setText(mToday.quality);

     //更新六天的数据
    for(int i=0;i<6;i++)
    {

       //日期和时间
       mWeekList[i]->setText(mDay[i].week);
       ui->lblWeek0->setText("昨天");
       ui->lblWeek1->setText("今天");
       ui->lblWeek2->setText("明天");
       QStringList ymdList = mDay[i].date.split("-");
       mDateList[i]->setText(ymdList[1] + "/" +ymdList[2]);

       //更新天气类型
       mTypeList[i]->setText(mDay[i].type);
       mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);

       //更新空气质量
       if(mDay[i].aqi >= 0 && mDay[i].aqi <= 50)
       {
           mAqiList[i]->setText("优");
           mAqiList[i]->setStyleSheet("background-color: rgb(121,184,0);");
       }else if(mDay[i].aqi >= 50 && mDay[i].aqi <= 100)
       {
           mAqiList[i]->setText("良");
           mAqiList[i]->setStyleSheet("background-color: rgb(255,187,23);");
       }else if(mDay[i].aqi >= 100 && mDay[i].aqi <= 150)
       {
           mAqiList[i]->setText("轻度");
           mAqiList[i]->setStyleSheet("background-color: rgb(255,87,97);");
       }else if(mDay[i].aqi >= 150 && mDay[i].aqi <= 200)
       {
           mAqiList[i]->setText("中度");
           mAqiList[i]->setStyleSheet("background-color: rgb(235,17,27);");
       }else if(mDay[i].aqi >= 200 && mDay[i].aqi <= 250)
       {
           mAqiList[i]->setText("重度");
           mAqiList[i]->setStyleSheet("background-color: rgb(170,0,0);");
       }else
       {
           mAqiList[i]->setText("严重");
           mAqiList[i]->setStyleSheet("background-color: rgb(110,0,0);");
       }

       //更新风力、风向
       mFxList[i]->setText(mDay[i].fx);
       mFlList[i]->setText(mDay[i].fl);
    }

}

//天气类型
void MainWindow::weatherType()
{
    //将控件添加到控件数组
    //星期和日期
    mWeekList <<ui->lblWeek0<<ui->lblWeek1<<ui->lblWeek2<<
        ui->lblWeek3<<ui->lblWeek4<<ui->lblWeek5;
    mDateList <<ui->lblDate0<<ui->lblDate1<<ui->lblDate2<<
        ui->lblDate3<<ui->lblDate4<<ui->lblDate5;

    //天气和天气图标
    mTypeList<< ui->lblType0<<ui->lblType1<<ui->lblType2<<
        ui->lblType3<<ui->lblType4<<ui->lblType5;
    mTypeIconList<< ui->lblTypeIcon0<<ui->lblTypeIcon1<<ui->lblTypeIcon2<<
        ui->lblTypeIcon3<<ui->lblTypeIcon4<<ui->lblTypeIcon5;

    //天气指数
    mAqiList<< ui->lblQuality0<<ui->lblQuality1<<ui->lblQuality2<<
        ui->lblQuality3<<ui->lblQuality4<<ui->lblQuality5;

    //风向和风力
    mFlList <<ui->lblFl0<<ui->lblFl1<<ui->lblFl2<<ui->lblFl3<<
        ui->lblFl4<<ui->lblFl5;
    mFxList <<ui->lblFx0<<ui->lblFx1<<ui->lblFx2<<ui->lblFx3<<
        ui->lblFx4<<ui->lblFx5;


    //天气对应的图标
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到暴雪",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到大暴雪",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
}





void MainWindow::on_btnSearch_clicked()
{
    QString  cityname = ui->leCity->text();
    getWeatherInfo(cityname);
    ui->leCity->clear();
}

void MainWindow::on_leCity_returnPressed()
{
    QString  cityname = ui->leCity->text();
    getWeatherInfo(cityname);
    ui->leCity->clear();
}

//重写父类的eventFilter方法
bool MainWindow::eventFilter(QObject *watched, QEvent *ev)
{
    if(watched == ui->lblHighCurve && ev->type() == QEvent::Paint)
    {
       paintHighCurve();
    }
    if(watched == ui->lblLowCurve && ev->type() == QEvent::Paint)
    {
       paintLowCurve();
    }

    return QWidget::eventFilter(watched,ev);
}


void MainWindow::paintHighCurve()   //绘制高温曲线
{
    QPainter painter(ui->lblHighCurve);

    painter.setRenderHint(QPainter::Antialiasing);

    //获取X坐标
    int PointX[6] = {0};
    for(int i = 0;i<6;i++)
    {
        PointX[i] = mWeekList[i]->pos().x()+mWeekList[i]->width()/2;
    }
    //获取Y坐标
    int PointY[6] = {0};
    int tempsum = 0;
    int tempaverage = 0;

    for(int i =0;i<6;i++)
    {
    tempsum += mDay[i].high;
    }
    tempaverage = tempsum/6;

    int yCenter = ui->lblHighCurve->height()/2 + 10;

     for(int i = 0;i<6;i++)
     {
        PointY[i] = yCenter - ((mDay[i].high-tempaverage) * HHH);
     }
    QPen pen = painter.pen();
    pen.setWidth(1);      //设置画笔的宽度
    pen.setColor(QColor(237,90,101)); //设置画笔的颜色

    painter.setPen(pen);
    painter.setBrush(QColor(192,44,56));  //画笔填充颜色


    // 画点，写温度数据
    for(int i = 0;i<6;i++)
    {
    painter.drawEllipse(QPoint(PointX[i],PointY[i]),POINT_RADIUS,POINT_RADIUS);

    painter.drawText(PointX[i]-X_pianyi,PointY[i]-Y_pianyi,QString::number(mDay[i].high) + "°");
    }
    //画线
    for(int i = 0;i<5;i++)
    {
        if(i == 0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }else {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }

        painter.drawLine(PointX[i],PointY[i],PointX[i+1],PointY[i+1]);
    }
}

void MainWindow::paintLowCurve()   //绘制高温曲线
{
    QPainter painter(ui->lblLowCurve);

    painter.setRenderHint(QPainter::Antialiasing);

    //获取X坐标
    int PointX[6] = {0};
    for(int i = 0;i<6;i++)
    {
        PointX[i] = mWeekList[i]->pos().x()+mWeekList[i]->width()/2;
    }
    //获取Y坐标
    int PointY[6] = {0};
    int tempsum = 0;
    int tempaverage = 0;

    for(int i =0;i<6;i++)
    {
    tempsum += mDay[i].high;
    }
    tempaverage = tempsum/6;

     int yCenter = ui->lblLowCurve->height()/2 - 25;

     for(int i = 0;i<6;i++)
     {
        PointY[i] = yCenter - ((mDay[i].low-tempaverage) * HHH);
     }
    QPen pen = painter.pen();
    pen.setWidth(1);      //设置画笔的宽度
    pen.setColor(QColor(27,167,132)); //设置画笔的颜色

    painter.setPen(pen);
    painter.setBrush(QColor(27,167,132));  //画笔填充颜色


    // 画点，写温度数据
    for(int i = 0;i<6;i++)
    {
    painter.drawEllipse(QPoint(PointX[i],PointY[i]),POINT_RADIUS,POINT_RADIUS);

    painter.drawText(PointX[i]-X_pianyi,PointY[i]-Y_pianyi,QString::number(mDay[i].low) + "°");
    //qDebug()<<"****------"<<mDay[i].high;
    }
    //画线
    for(int i = 0;i<5;i++)
    {
        if(i == 0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }else {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }

        painter.drawLine(PointX[i],PointY[i],PointX[i+1],PointY[i+1]);
    }
}




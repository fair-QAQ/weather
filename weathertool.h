#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H
#include <QString>
#include <QMap>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QCoreApplication>
#include<QtDebug>


class weatherTool
{
private:
    static QMap<QString,QString>mCityMap;

    static void initCityMap()
    {
    QString filepath = ":/citycode.json";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray json = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc =  QJsonDocument::fromJson(json,&err);

    if(err.error != QJsonParseError::NoError)
    {
        qDebug()<<"文件转出来的json文件格式有误！！";
        return;
     }
    if(!doc.isArray())
    {
       qDebug()<<"doc不是array格式";
        return;
    }

    QJsonArray cities = doc.array();
    for(int i=0; i<cities.size(); i++)
    {
        QString city = cities[i].toObject().value("city_name").toString();
        QString code = cities[i].toObject().value("city_code").toString();

        if(code.size()>0)
        {
            mCityMap.insert(city,code);
        }
    }
    }

 public:
    static QString GetCity(QString cityName)
    {

        if(mCityMap.isEmpty())
        {
           initCityMap();
        }
     QMap<QString, QString>::iterator it =  mCityMap.find(cityName);
     if(it == mCityMap.end())
          it =  mCityMap.find(cityName + "市");

     if(it != mCityMap.end())
         return it.value();


     return "";
    }
};

QMap<QString,QString> weatherTool::mCityMap = { };

#endif // WEATHERTOOL_H

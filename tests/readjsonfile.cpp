#include "readjsonfile.h"

ReadJsonFile::ReadJsonFile()
{
    QString val;
    QFile file;
    QString testfilePath = "../forecast.json";
    file.setFileName(testfilePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    m_jsonObj = d.object();
}

QJsonObject ReadJsonFile::readJson() //TODO müsste irgendwie getTestData heißen oder so
{
    return m_jsonObj;
}

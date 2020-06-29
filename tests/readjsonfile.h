#ifndef READJSONFILE_H
#define READJSONFILE_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

class ReadJsonFile
{

public:
    ReadJsonFile();

    QJsonObject readJson();
    
private:
    QJsonObject m_jsonObj;
};

#endif // READJSONFILE_H 

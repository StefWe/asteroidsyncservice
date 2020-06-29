#include <QCoreApplication>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QDebug>

#include "../asteroidsyncserviced/openweathermapparser.h"
#include "readjsonfile.h"
#include "gmock/gmock.h"

using namespace testing;

class OWMParserTest: public Test {
public:
    OpenWeatherMapParser owmparser;
    ReadJsonFile readjsonfile;

    QJsonObject rootObj;
};

TEST_F(OWMParserTest, RetainWeatherCityName) {
    ASSERT_THAT(owmparser.getCity(readjsonfile.readJson().value("city").toObject()), Eq("Ludwigsburg"));
}

TEST_F(OWMParserTest, RetainWeatherID) {
    ASSERT_THAT(owmparser.getWeatherId(readjsonfile.readJson().value("list").toArray()), Eq(QList<short>() << 800 << 800 << 800 << 800 << 800));
}

TEST_F(OWMParserTest, RetainMinTemp) {
    ASSERT_THAT(owmparser.getTempMin(readjsonfile.readJson().value("list").toArray()), Eq(QList<short>() << 272 << 270 << 271 << 272 << 276));
}

TEST_F(OWMParserTest, RetainMaxTemp) {
    ASSERT_THAT(owmparser.getTempMax(readjsonfile.readJson().value("list").toArray()), Eq(QList<short>() << 275 << 280 << 283 << 284 << 285));
}

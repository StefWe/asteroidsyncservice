/*
 * Copyright (C) 2018 - Florent Revest <revestflo@gmail.com>
 *               2016 - Andrew Branson <andrew.branson@jollamobile.com>
 *                      Ruslan N. Marchenko <me@ruff.mobi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusinterface.h"
#include "watchesmanager.h"
#include "libasteroid/watch.h"

#include <QDBusConnection>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>

/* Watch Interface */

DBusWatch::DBusWatch(Watch *watch, WatchesManager* wm, QObject *parent): QObject(parent), m_watch(watch), m_wm(wm)
{
    m_screenshotService = wm->screenshotService();
    m_weatherService = wm->weatherService();
    m_batteryService = wm->batteryService();
    m_timeService = wm->timeService();
    m_notificationService = wm->notificationService();

    connect(m_batteryService, SIGNAL(ready()), this, SIGNAL(BatteryServiceReady()));    
    connect(m_batteryService, SIGNAL(levelChanged(quint8)), this, SIGNAL(LevelChanged(quint8)));
    connect(m_timeService, SIGNAL(ready()), this, SLOT(onTimeServiceReady()));
    connect(m_notificationService, SIGNAL(ready()), this, SLOT(onNotifyServiceReady()));
    connect(m_screenshotService, SIGNAL(ready()), this, SLOT(onScreenshotServiceReady()));
    connect(m_screenshotService, SIGNAL(progressChanged(unsigned int)), this, SIGNAL(ProgressChanged(unsigned int)));
    connect(m_screenshotService, SIGNAL(screenshotReceived(QByteArray)), this, SIGNAL(ScreenshotReceived(QByteArray)));
    connect(m_weatherService, SIGNAL(ready()), this, SLOT(onWeatherServiceReady()));
    connect(wm, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    m_nam = new QNetworkAccessManager(this);
    m_wmp = new OpenWeatherMapParser(this);
}

void DBusWatch::onDisconnected()
{
    m_timeServiceReady = false;
    emit TimeServiceChanged();

    m_notifyServiceReady = false;
    emit NotifyServiceChanged();

    m_screenshotServiceReady = false;
    emit ScreenshotServiceChanged();

    m_weatherServiceReady = false;
    emit WeatherServiceChanged();
}

void DBusWatch::SelectWatch()
{
    m_wm->setDevice(m_watch);
}

QString DBusWatch::Address() const
{
    return m_watch->getAddress().toString();
}

QString DBusWatch::Name() const
{
    return m_watch->getName();
}

quint8 DBusWatch::BatteryLevel()
{
    return m_batteryService->level();
}

void DBusWatch::RequestScreenshot()
{
    m_screenshotService->requestScreenshot();
}

void DBusWatch::onTimeServiceReady()
{
    m_timeServiceReady = true;
    emit TimeServiceChanged();
}

bool DBusWatch::StatusTimeService()
{
    return m_timeServiceReady;
}

void DBusWatch::onNotifyServiceReady()
{
    m_notifyServiceReady = true;
    emit NotifyServiceChanged();
}

bool DBusWatch::StatusNotifyService()
{
    return m_notifyServiceReady;
}

void DBusWatch::SetTime(QDateTime t)
{
    m_timeService->setTime(t);
}

void DBusWatch::SetVibration(QString v)
{
    m_notificationService->setVibration(v);
}

void DBusWatch::SendNotify(unsigned int id, QString appName, QString icon, QString body, QString summary)
{
    m_notificationService->insertNotification("", id, appName, icon, body, summary, NotificationService::Strong);
}

void DBusWatch::onScreenshotServiceReady()
{
    m_screenshotServiceReady = true;
    emit ScreenshotServiceChanged();
}

bool DBusWatch::StatusScreenshotService()
{
    return m_screenshotServiceReady;
}

void DBusWatch::onWeatherServiceReady()
{
    m_weatherServiceReady = true;
    emit WeatherServiceChanged();
}

bool DBusWatch::StatusWeatherService()
{
    return m_weatherServiceReady;
}

void DBusWatch::SetWeatherLocation(const QString lat, const QString lng)
{
    owmRequest(lat, lng);
}

void DBusWatch::owmRequest(const QString lat, const QString lng) const
{
    QString owmApiKey = "b1af1d2053458fb4ab724f038ed499aa";
    QUrl url;
    url.setUrl("http://api.openweathermap.org/data/2.5/forecast");

    QUrlQuery query;
    query.addQueryItem("lat", lat);
    query.addQueryItem("lon", lng);
    query.addQueryItem("appid", owmApiKey);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply* reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &DBusWatch::onOwmReplyFinished);
}

void DBusWatch::onOwmReplyFinished()
{
    QJsonObject rootObj;
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError)  {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        rootObj = document.object();
    } else {
        qDebug() << "Network error" << reply->errorString();
        return;
    }

    m_wmp->prepareData(rootObj);
    m_weatherService->setCity(m_wmp->getCity());
    qDebug() << "WeatherID" << m_wmp->getWeatherId();
    m_weatherService->setIds(m_wmp->getWeatherId());
    qDebug() << "Min Temp" << m_wmp->getTempMin();
    m_weatherService->setMinTemps(m_wmp->getTempMin());
    qDebug() << "Max Temp" << m_wmp->getTempMax();
    m_weatherService->setMaxTemps(m_wmp->getTempMax());
}

/* Manager Interface */

DBusInterface::DBusInterface(WatchesManager *wm, QObject *parent) : QObject(parent)
{
    m_watchesManager = wm;

    QDBusConnection::sessionBus().registerService("org.asteroidsyncservice");
    QDBusConnection::sessionBus().registerObject("/org/asteroidsyncservice/Manager", this, QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);

    foreach (Watch *watch, m_watchesManager->watches())
        watchAdded(watch);

    connect(m_watchesManager, &WatchesManager::watchAdded, this, &DBusInterface::watchAdded);
    connect(m_watchesManager, &WatchesManager::watchRemoved, this, &DBusInterface::watchRemoved);
    connect(m_watchesManager, SIGNAL(currentWatchChanged()), this, SIGNAL(SelectedWatchChanged()));
    connect(m_watchesManager, SIGNAL(connected()), this, SIGNAL(SelectedWatchConnectedChanged()));
    connect(m_watchesManager, SIGNAL(disconnected()), this, SIGNAL(SelectedWatchConnectedChanged()));
}

QString DBusInterface::Version()
{
    return QStringLiteral(VERSION);
}

QList<QDBusObjectPath> DBusInterface::ListWatches()
{
    QList<QDBusObjectPath> ret;

    foreach (const QString &address, m_dbusWatches.keys())
        ret.append(QDBusObjectPath("/org/asteroidsyncservice/" + address));

    return ret;
}

QDBusObjectPath DBusInterface::SelectedWatch()
{
    Watch *cur = m_watchesManager->currentWatch();
    if(cur) {
        QString address = cur->getAddress().toString().replace(":", "_");
        return QDBusObjectPath("/org/asteroidsyncservice/" + address);
    } else
        return QDBusObjectPath("/");
}

bool DBusInterface::SelectedWatchConnected()
{
    return m_watchesManager->isConnected();
}

void DBusInterface::watchAdded(Watch *watch)
{
    QString address = watch->getAddress().toString().replace(":", "_");
    if (m_dbusWatches.contains(address))
        return;

    DBusWatch *dbusWatch = new DBusWatch(watch, m_watchesManager, this);
    m_dbusWatches.insert(address, dbusWatch);
    QDBusConnection::sessionBus().registerObject("/org/asteroidsyncservice/" + address, dbusWatch, QDBusConnection::ExportAllContents);

    emit WatchesChanged();
}

void DBusInterface::watchRemoved(Watch *watch)
{
    QString address = watch->getAddress().toString().replace(":", "_");

    QDBusConnection::sessionBus().unregisterObject("/org/asteroidsyncservice/" + address);
    m_dbusWatches.remove(address);

    emit WatchesChanged();
}


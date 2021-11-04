/*
 *   This file is part of Qt-SSE-Demo.
 *
 *   Qt-SSE-Demo is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Qt-SSE-Demo is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Qt-SSE-Demo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QFile>
#include <QSSlKey>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include "networkmanager.h"
#include "configurationsection.h"
#include <QSsl>
Network::Manager* Network::Manager::m_instance = nullptr;

/**
 * @file networkmanager.cpp
 * @author Dylan Van Assche
 * @date 21 Jan 2019
 * @brief Network::Manager facade constructor
 * @package Network
 * @private
 * Constructs a Network::Manager facade to access the network using the HTTP protocol.
 * The Network::Manager facade makes network access in Qt abstract from the underlying library (QNetworkAccessManager, libCurl, ...).
 */
Network::Manager::Manager(QObject* parent) : QObject(parent)
{
    // Initiate a new QNetworkAccessManager
    this->setQNAM(new QNetworkAccessManager(this));
    QNetworkConfigurationManager QNAMConfig;
    this->QNAM()->setConfiguration(QNAMConfig.defaultConfiguration());

    // Connect QNetworkAccessManager signals
    connect(this->QNAM(),
        SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)),
        this,
        SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(this->QNAM(),
        SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
        this,
        SIGNAL(sslErrorsReceived(QNetworkReply*, QList<QSslError>)));
    connect(this->QNAM(),
        SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
        this,
        SLOT(handleSslErrors(QNetworkReply*, QList<QSslError>)));
    connect(this->QNAM(),
        SIGNAL(finished(QNetworkReply*)),
        this,
        SLOT(streamFinished(QNetworkReply*)));

    // Init retries counter
    m_retries = 0;
}

/**
 * @file networkmanager.cpp
 * @author Dylan Van Assche
 * @date 21 Jan 2019
 * @brief Get a Network::Manager instance
 * @param QObject *parent = nullptr
 * @return Network::Manager *manager
 * @package Network
 * @public
 * Constructs a Network::Manager if none exists and returns the instance.
 */
Network::Manager* Network::Manager::getInstance()
{
    if (m_instance == nullptr) {
        qDebug() << "Creating new Network::Manager";
        m_instance = new Manager();
    }
    return m_instance;
}

// Invokers
/**
 * @file networkmanager.cpp
 * @author Dylan Van Assche
 * @date 21 Jan 2019
 * @brief Get a resource
 * @param const QUrl &url
 * @param QObject *caller
 * @package Network
 * @public
 * Retrieves a certain resource from the given QUrl &url using a GET request.
 * The result as a QNetworkReply *reply will be available through the Qt event system.
 * Implement the customEvent() method in your QObject to receive the data.
 */
void Network::Manager::getResource(const QUrl& url)
{
    qDebug() << "GET resource:" << url;
    QNetworkRequest request = this->prepareRequest(url);

    m_reply = this->QNAM()->get(request);
    
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(streamReceived()));
    connect(this, SIGNAL(additionResultRecieved(int)), this, SLOT(handleAdditionResultRecieved(int)));
    connect(this, SIGNAL(subtractionResultRecieved(int)), this, SLOT(handleSubtractionResultRecieved(int)));
    
    
}

void Network::Manager::streamFinished(QNetworkReply* reply)
{
    qDebug() << "Stream finished:" << reply->url();
    qDebug() << "Reconnecting...";
    if (m_retries < MAX_RETRIES) {
        m_retries++;
        this->getResource(reply->url());
    }
    else {
        qCritical() << "Unable to reconnect, max retries reached";
    }
}

void Network::Manager::streamReceived()
{
    //string to hold response
    QString response = QString(m_reply->readAll()).simplified().replace("data: ", "");
    qDebug() << "Received event from stream";
    qDebug() << response;
    qDebug() << "-----------------------------------------------------";
    m_retries = 0;

    invokeFunc();
    //QThread::sleep(2);
    invokeFunc2();
    parseResponse(response);

}

// Helpers
/**
 * @file networkmanager.cpp
 * @author Halemo w halem 3mmak ya mark
 * @date 21 Jan 2019
 * @brief Prepare the HTTP request
 * @param const QUrl &url
 * @return QNetworkRequest request
 * @package Network
 * @public
 * Everytime a HTTP request has been made by the user it needs several default headers to complete it's mission.
 * The prepareRequest method just does that, it adds the Accept, User-Agent header to the request and allows redirects.
 */
QNetworkRequest Network::Manager::prepareRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    //request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, false);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    
    //config.setProtocol(QSsl::TlsV1_2);

    //load configurations
    ConfigurationSection::start();
    QString certifactePath = ConfigurationSection::readConfigurationValue("AppSettings/CertificatePath");
    QString caCertifactePath = ConfigurationSection::readConfigurationValue("AppSettings/CaCertificatePath");
    QString keyPath = ConfigurationSection::readConfigurationValue("AppSettings/KeyPath");

    //set certificates & private keys
    config.setLocalCertificate(QSslCertificate::fromPath(certifactePath.toStdString().c_str())[0]);
    config.setCaCertificates(QSslCertificate::fromPath(caCertifactePath));
    QFile privateKey(keyPath);
    privateKey.open(QFile::ReadOnly);
    QSslKey key(privateKey.readAll(), QSsl::Rsa);
    privateKey.close();
    config.setPrivateKey(key);

    //prepare the request to be send
    request.setSslConfiguration(config);
    request.setRawHeader(QByteArray("Accept"), QByteArray(ACCEPT_HEADER));
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Events shouldn't be cached
    return request;
}

// Getter & Setters
/**
 * @file networkmanager.cpp
 * @author Dylan Van Assche
 * @date 21 Jan 2019
 * @brief Gets the QNAM instance
 * @return QNetworkAccessManager *QNAM
 * @package Network
 * @public
 * Gets the QNetworkAccessManager instance.
 */
QNetworkAccessManager* Network::Manager::QNAM() const
{
    return m_QNAM;
}

/**
 * @file networkmanager.cpp
 * @author Dylan Van Assche
 * @date 21 Jan 2019
 * @brief Sets the QNAM instance
 * @param QNetworkAccessManager *value
 * @package Network
 * @public
 * Sets the QNetworkAccessManager instance.
 */
void Network::Manager::setQNAM(QNetworkAccessManager* value)
{
    m_QNAM = value;
}

void Network::Manager::handleSslErrors(QNetworkReply* reply, QList<QSslError> sslError)
{
    QList<QSslError> errorsThatCanBeIgnored;
    errorsThatCanBeIgnored << QSslError(QSslError::HostNameMismatch);
    errorsThatCanBeIgnored << QSslError(QSslError::SelfSignedCertificate);

    reply->ignoreSslErrors(errorsThatCanBeIgnored);

    for each (auto error in sslError)
    {
        qDebug() << "Error: " << error.errorString();
    }
}

void Network::Manager::handleAdditionResultRecieved(int result)
{
    qDebug() << "------------------------";
    qDebug() << "Addition operation occured, the result is " << result;
}

void Network::Manager::handleSubtractionResultRecieved(int result)
{
    qDebug() << "------------------------";
    qDebug() << "Subtraction operation occured, the result is " << result;
}

//Alert!!!! for debugging onlyyy
void print(QList<QString>& l, QString word)
{
    qDebug() << word;
    for (auto i : l)
    {
        qDebug().noquote() << i << ' ';
    }
}

QString removeRubbish(QString s)
{

    QString res = s.remove("\\")
        .remove("\"")           
        .remove("}")            
        .remove("{");
    return res;
}


void Network::Manager::parseResponse(QString response)
{
    if (response.isEmpty() || !response.contains("FunctionName"))
        return;
    //int funcIndex = response.indexOf("FunctionName");
    QList<QString> keyValues = response.split(",");
    //print(keyValues, "after splitting ,");
    QHash<QString, QString> mp;
    for (auto keyValue : keyValues)
    {
        QList<QString>keyValuePair = keyValue.split(":");
        //print(keyValuePair, "AFter splitting : ");
        keyValuePair[0] = removeRubbish(keyValuePair[0]);
        keyValuePair[1] = removeRubbish(keyValuePair[1]);
        //qDebug().noquote() << "keyvlauepair[0]: " << keyValuePair[0];
        //qDebug().noquote() << "keyvlauepair[1]: " << keyValuePair[1];
        mp.insert(keyValuePair[0], keyValuePair[1]);
    }

    //clean the response from additional symbols
    //mp["FunctionName"] = mp["FunctionName"].remove("}").remove("\\");
    if (mp["FunctionName"] == "ADD")
    {
        
        emit additionResultRecieved(mp["Result"].toInt());
    }
    else if (mp["FunctionName"] == "SUBTRACT")
    {
        emit subtractionResultRecieved(mp["Result"].toInt());
    }
    else {
        qDebug() << "Server sent an unexpected response!";
        qDebug() << "-------------------------------------";
    }
}

void Network::Manager::invokeFunc()
{
    QNetworkAccessManager* mgr = this->QNAM();
    const QUrl url(QStringLiteral("https://localhost:3000/"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["num"] = "5";
    obj["num1"] = "3";
    obj["op"] = "add";
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    // or
    // QByteArray data("{\"key1\":\"value1\",\"key2\":\"value2\"}");
    QNetworkReply* reply = mgr->post(request, data);
    
    //qDebug() << "Method is invoked";
    
}

void Network::Manager::invokeFunc2()
{
    QNetworkAccessManager* mgr = this->QNAM();
    const QUrl url(QStringLiteral("https://localhost:3000/"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["num"] = "5";
    obj["num1"] = "3";
    obj["op"] = "diff";
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    // or
    // QByteArray data("{\"key1\":\"value1\",\"key2\":\"value2\"}");
    QNetworkReply* reply = mgr->post(request, data);

    //qDebug() << "Method 2 is invoked";
}
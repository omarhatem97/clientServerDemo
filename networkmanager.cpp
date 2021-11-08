#include <QFile>
#include <QSSlKey>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include "networkmanager.h"
#include "configurationsection.h"
#include <QSsl>
#include<QUuid>
//#include <qInfo>
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
Network::Manager::Manager(QObject* parent) 
	: QObject(parent)
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
	m_flag = 0;
}
/* Van Assche
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
	chId = QUuid::createUuid().toString().mid(1, 36);
	//QNetworkRequest request = this->prepareRequest(url);
	QNetworkRequest request = QNetworkRequest(QUrl("http://localhost:3000/"+ chId + "/listen"));

	//Debugging
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	QJsonObject obj;
	obj["num"] = "5";
	obj["num1"] = "3";
	obj["op"] = "diff";
	QJsonDocument doc(obj);
	QByteArray data = doc.toJson();

	//End Debugging

	//m_reply = this->QNAM()->post(request, data);
	m_reply = this->QNAM()->get(request);
	connect(m_reply, SIGNAL(readyRead()), this, SLOT(streamReceived()));
	//connect(this, SIGNAL(readyRead()), this, SLOT(()));
}

void Network::Manager::streamFinished(QNetworkReply* reply)
{
	qDebug() << "Stream finished:" << reply->url();
	qDebug() << "Reconnecting...";
	if (m_retries < MAX_RETRIES) {
		m_retries++;
		//this->getResource(reply->url());
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

	if (!m_flag)
	{
		//Make a Post request to the server only once
		QNetworkRequest request =QNetworkRequest(QUrl("http://localhost:3000/" + chId + "/send"));
		//Debugging
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		QJsonObject obj;
		obj["num"] = "5";
		obj["num1"] = "3";
		obj["op"] = "diff";
		QJsonDocument doc(obj);
		QByteArray data = doc.toJson();
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
		this->QNAM()->post(request, data);
	}

	m_flag = 1;
	
	
	//post();
	//invokeFunc();
	////QThread::sleep(2);
	//invokeFunc2();
	//parseResponse();

}

// Helpers
/**
 * @file networkmanager.cpp
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

void Network::Manager::handleAdditionResultRecieved()
{
	qDebug() << "------------------------";
	qDebug() << "Addition operation occured, the result is " << m_postReply->readAll();
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


void Network::Manager::parseResponse()
{
	QString response = QString(m_reply->readAll()).simplified().replace("data: ", "");
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

		qInfo() << "Addition operation occured and the Result is :" << (mp["Result"].toInt());
	}
	else if (mp["FunctionName"] == "SUBTRACT")
	{
		qInfo() << "Subtraction operation occured and the Result is :" <<(mp["Result"].toInt());
	}
	else {
		qDebug() << "Server sent an unexpected response!";
		qDebug() << "-------------------------------------";
	}
}

void Network::Manager::post()
{

	//if (m_flag) return;
	//m_flag = 1;
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
	m_postReply = mgr->post(request, data);

	//connect(m_postReply, &Network::Manager::additionResultRecieved, this, &Network::Manager::handleAdditionResultRecieved);
	//connect(m_postReply, &Network::Manager::subtractionResultRecieved, this, &Network::Manager::handleSubtractionResultRecieved);
	connect(m_postReply, &QNetworkReply::readyRead, this, &Network::Manager::parseResponse);
}

bool Network::Manager::connectionEstablished()
{
	return m_flag ? true : false;
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
	obj["num"]	= "5";
	obj["num1"] = "3";
	obj["op"]	= "diff";
	QJsonDocument doc(obj);
	QByteArray data = doc.toJson();
	// or
	// QByteArray data("{\"key1\":\"value1\",\"key2\":\"value2\"}");
	QNetworkReply* reply = mgr->post(request, data);

	//qDebug() << "Method 2 is invoked";
}


	
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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtNetwork/QAbstractNetworkCache>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslError>
#include <QUrl>

#define ACCEPT_HEADER "text/event-stream"
#define USER_AGENT "Qt SSE demo"
#define MAX_RETRIES 3

 // Singleton pattern
namespace Network {

	class Manager : public QObject
	{
		Q_OBJECT
	public:
		static Manager* getInstance();
		//void parseResponse();
		void post();
		bool connectionEstablished();
		void setOp  (QString op);
		void setNum (QString num);
		void setNum1(QString num1);
		QString getOp();
		QString getNum ();
		QString getNum1();
		QNetworkReply* m_reply;
	signals:
		QList<QSslError> sslErrorsReceived(QNetworkReply* reply, QList<QSslError> sslError);
		QNetworkAccessManager::NetworkAccessibility networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility state);
		void additionResultRecieved(int result);
		void subtractionResultRecieved(int result);
	public slots:
		void getResource(QUrl& url);
		void handleSslErrors(QNetworkReply* reply, QList<QSslError> sslError);
		void handleAdditionResultRecieved();
		void handleSubtractionResultRecieved(int result);
		void parseResponse();
	private slots:
		void streamFinished(QNetworkReply* reply);
		void streamReceived();

	private:
		qint16 m_retries;
		QNetworkReply* m_postReply;
		QNetworkAccessManager* m_QNAM;
		static Manager* m_instance;
		bool m_flag;
		explicit Manager(QObject* parent = nullptr);
		QNetworkRequest prepareRequest(const QUrl& url);
		QNetworkAccessManager* QNAM() const;
		void setQNAM(QNetworkAccessManager* value);
		void setChId();
		static Manager* manager();
		static void setManager(const Manager* manager);
		QString chId;
		
		QString m_op, m_num, m_num1;
		QString m_url;

		// post ---> this.qnam.post(request, data)
		
		void invokeFunc();
		void invokeFunc2();
	};
}

#endif // NETWORKMANAGER_H

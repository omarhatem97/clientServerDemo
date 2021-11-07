#include "configurationsection.h"
#include <QCoreApplication>

QMap<QString, QString> ConfigurationSection::m_Configurations;
QString ConfigurationSection::m_ConfigFile = QString("configuration.ini");

void ConfigurationSection::start() {
  //Open the configuration file in INI mode to the application settings object
  QSettings settings(ConfigurationSection::getPathConfiguration(), QSettings::IniFormat);
  QStringList keys = settings.allKeys();

  for (QString key : keys) {
      //qDebug() << key << settings.value(key).toString();
      m_Configurations.insert(key, settings.value(key).toString());
  }
}

QString ConfigurationSection::readConfigurationValue(QString key) {
  //Query the given key value from the application settings object
  QString value = m_Configurations.value(key, "");
  if(value.contains("/"))
      value = QDir::fromNativeSeparators(value);

  return value;
}


QString ConfigurationSection::getPathConfiguration() {
  QString path;
  path = qApp->applicationDirPath() + "/" + m_ConfigFile;
  return path;
}




void ConfigurationSection::end() {
  m_Configurations.clear();
}


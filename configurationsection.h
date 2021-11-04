#ifndef CONFIGURATIONSECTION_H
#define CONFIGURATIONSECTION_H

#include <QMap>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QString>


class ConfigurationSection {
public:   

  /**
   * @brief Gets configuration value by key.
   * @return returns the configuration value string.
   */
  static QString readConfigurationValue(QString key);

  /**
   * @brief Alters specified configuration value in configuration writable.
   */
  static void writeConfigurationValue(QString key, QString value);

  /**
   * @brief Gets the folder path where you have to get the configuration.
   * @return path folder path to configuration file
   */
  static QString getPathConfiguration();

  /**
   * @brief Starts the ConfigurationSection and load associated files.
   */
  static void start();

  /**
  * @brief Ends the ConfigurationSection.
  */
  static void end();

 

private:
  static QString m_ConfigFile;
  static QMap<QString, QString> m_Configurations;
};

#endif // CONFIGURATIONSECTION_H


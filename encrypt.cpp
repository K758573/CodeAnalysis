//
// Created by Shiro on 2023/7/20.
//
#include "src/CipherUtils.h"
#include <QSettings>
#include <iostream>

const QString CONFIG_FILE_NAME("database.ini");
const QString KEY_DATABASE_USERNAME("username");
const QString KEY_DATABASE_PASSWORD("password");

int main()
{
  QSettings settings(CONFIG_FILE_NAME, QSettings::IniFormat);
  QString username = settings.value(KEY_DATABASE_USERNAME).toString();
  settings.setValue(KEY_DATABASE_USERNAME, QString::fromStdString(CipherUtils::Encrypt(username.toStdString())));
  QString password = settings.value(KEY_DATABASE_PASSWORD).toString();
  settings.setValue(KEY_DATABASE_PASSWORD, QString::fromStdString(CipherUtils::Encrypt(password.toStdString())));
  std::cout << "数据库账户密码加密完成" << std::endl;
  return 0;
}
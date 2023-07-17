//
// Created by Shiro on 2023/7/14.
//

#include <QStringList>
#include "RiskFunctionDb.h"
#include "src/Log.h"
#include <QDebug>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QFile>
#include <QSettings>

const QString RISK_FUNCTION_TABLE_NAME = "risk_function";
const QString RISK_FUNCTION_FIELD_NAME = "name";
const QString RISK_FUNCTION_FIELD_DESCRIPTION = "description";
const QString RISK_FUNCTION_FIELD_SUGGESTION = "suggestion";
const QString RISK_FUNCTION_FIELD_LEVEL = "level";

const QString CONFIG_FILE_NAME("database.ini");
const QString KEY_DATABASE_HOSTNAME("hostname");
const QString KEY_DATABASE_USERNAME("username");
const QString KEY_DATABASE_PASSWORD("password");
const QString KEY_DATABASE_DATABASE("database");


RiskFunctionDB::RiskFunctionDB()
{
  //连接数据库
  db = QSqlDatabase::addDatabase("QMYSQL");
  QSettings settings(CONFIG_FILE_NAME, QSettings::IniFormat);
  QString hostname = settings.value(KEY_DATABASE_HOSTNAME).toString();
  QString username = settings.value(KEY_DATABASE_USERNAME).toString();
  QString password = settings.value(KEY_DATABASE_PASSWORD).toString();
  QString database = settings.value(KEY_DATABASE_DATABASE).toString();
  db.setHostName(hostname);
  db.setUserName(username);
  db.setPassword(password);
  db.setDatabaseName(database);
  if (!db.open()) {
    LOG("数据库初始化失败");
  }
}

RiskFunction RiskFunctionDB::selectByName(const QString &name) const
{
  QSqlTableModel model(nullptr, db);
  model.setTable(RISK_FUNCTION_TABLE_NAME);
  model.setFilter(QString("name='%1'").arg(name));
  model.select();
  return RiskFunction::fromSqlRecord(model.record(0));
}

RiskFunction RiskFunction::fromSqlRecord(const QSqlRecord &record)
{
  if (record.isEmpty()) {
    LOG("查询了数据库中不存在的风险函数");
  }
  return RiskFunction{
      record.value(RISK_FUNCTION_FIELD_NAME).value<QString>(), record.value(RISK_FUNCTION_FIELD_LEVEL).value<int>(),
      record.value(RISK_FUNCTION_FIELD_DESCRIPTION).value<QString>(),
      record.value(RISK_FUNCTION_FIELD_SUGGESTION).value<QString>()
  };
}


//void RiskFunctionDB::insertRecord(std::initializer_list<RiskFunction> funcs)
//{
//  QSqlTableModel model(nullptr, db);
//  model.setTable(RiskFunction::RISK_FUNCTION_TABLE_NAME);
//  model.select();
//  QSqlRecord record = model.record();
//  QFile file("out.txt");
//  file.open(QIODevice::ReadOnly);
//  QTextStream qts(&file);
//  QString name;
//  QString level;
//  QString suggestion;
//  qts.setPadChar('\t');
//  while (!qts.atEnd()) {
//    const QString &qstr = qts.readLine();
//    const QStringList &list = qstr.split('\t');
//    name = list[0];
//    level = list[1];
//    suggestion = list[2];
//    //    qts >> name >> level >> suggestion;
//    record.setValue(RiskFunction::RISK_FUNCTION_FIELD_NAME, name);
//    if (level.contains("最危险")) {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 100);
//    } else if (level.contains("稍小")) {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 40);
//    } else if (level.contains("很危险")) {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 80);
//    } else if (level.contains("危险")) {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 60);
//    } else if (level.contains("中等危险")) {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 30);
//    } else if (level.contains("低危险")) {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 10);
//    } else {
//      record.setValue(RiskFunction::RISK_FUNCTION_FIELD_LEVEL, 0);
//    }
//    record.setValue(RiskFunction::RISK_FUNCTION_FIELD_SUGGESTION, suggestion);
//    record.setValue(RiskFunction::RISK_FUNCTION_FIELD_DESCRIPTION, "");
//    model.insertRecord(0, record);
//    qDebug() << model.submit();
//  }
//}
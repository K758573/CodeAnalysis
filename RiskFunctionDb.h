//
// Created by Shiro on 2023/7/14.
//

#ifndef CODEANALYSIS_RISKFUNCTIONDB_H
#define CODEANALYSIS_RISKFUNCTIONDB_H

#include <QtSql/QSqlDatabase>

class RiskFunction
{
public:
  QString name;
  int level;
  QString description;
  QString suggestion;
  
  static RiskFunction fromSqlRecord(const QSqlRecord &record);
};

class RiskFunctionDB
{
public:
  RiskFunctionDB();
  
//  void insertRecord(std::initializer_list<RiskFunction> funcs);
  //根据函数名查询数据
  [[nodiscard]] RiskFunction selectByName(const QString &name) const;

private:
  QSqlDatabase db;
};


#endif //CODEANALYSIS_RISKFUNCTIONDB_H

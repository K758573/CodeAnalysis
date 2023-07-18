//
// Created by Shiro on 2023/7/18.
//

#ifndef CODEANALYSIS_REPORTGENERATOR_H
#define CODEANALYSIS_REPORTGENERATOR_H

#include <vector>
#include "RiskFunctionDb.h"
#include "AstParser.h"
/**
 * 报告生成器，根据使用到的风险函数集合，生成一份word文档，说明使用到的风险函数信息
 */
class ReportGenerator
{
public:
  explicit ReportGenerator(std::vector<const clang::CallExpr *> &callees, RiskFunctionDB &db,
                           CodeAnalysis::MatchHandler &result);
  
  /**
   * 生成一份报告
   * @return 生成成功返回true
   */
  bool generate();

private:
  std::vector<const clang::CallExpr *> callees_;
  RiskFunctionDB &db_;
  CodeAnalysis::MatchHandler& result_;
};


#endif //CODEANALYSIS_REPORTGENERATOR_H

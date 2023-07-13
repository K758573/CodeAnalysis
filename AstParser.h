//
// Created by Shiro on 2023/7/11.
//

#ifndef CODEANALYSIS_ASTPARSER_H
#define CODEANALYSIS_ASTPARSER_H

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclarationName.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>


#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace CodeAnalysis {

//保存AST树中的结构体，函数，变量，枚举等
class ASTRecord{
  std::vector<const clang::FunctionDecl *> functions_;
//  std::vector<const clang::VarDecl *> functions_;

};

class MatchHandler :
    public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
  //从文件名到最外侧声明的映射
  std::map<std::string, std::vector<const clang::NamedDecl *>> map_file_decl_;
  
  //统计函数调用
  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
  
  //提取函数和函数内的变量
  std::map<const clang::NamedDecl *, std::vector<const clang::NamedDecl *>>
  extractFunctionsAndVars(std::string filepath);
  
//  std::vector<const clang::VarDecl *> getVarDeclsByFuncName(const std::string &file, const std::string &func_name);
    ///获取类型字符串
    std::string getTypeAsString(const clang::NamedDecl *named_decl);
};


class ASTParser
{
public:
  ///通过命令行参数初始化
  ASTParser(int argc, const char **argv);
  
  ///初始化后，应该先调用该函数，生成文件的AST树
  void initASTs(const std::vector<std::string> &files);
  

  
  ///对所有AST进行语法检查
  std::string syntaxCheck();
  
  MatchHandler &result()
  {
    return handler_;
  }

private:
  //解析ast
  void parseAST();
  
  
public:
  std::vector<std::unique_ptr<clang::ASTUnit>> asts;

private:
  clang::Expected<clang::tooling::CommonOptionsParser> common_options_parser_;
  
  llvm::raw_svector_ostream stream_string_buffer_;
  
  clang::SmallVector<char> diagnostic_message_buffer_;
  
  MatchHandler handler_;
  
  clang::ast_matchers::MatchFinder finder_;
};


} // CodeAnalysis

#endif //CODEANALYSIS_ASTPARSER_H

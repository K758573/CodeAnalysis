//
// Created by Shiro on 2023/7/11.
//

#include "AstParser.h"
#include "CodeAnalysis.h"
#include <filesystem>
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace llvm::cl;

const std::string NAMED_DECL_FUNC = "NAMED_DECL_FUNC";
const std::string NAMED_DECL_VAR_GLOBAL = "NAMED_DECL_VAR_GLOBAL";
const std::string NAMED_DECL_OUTERMOST_ALL = "NAMED_DECL_OUTERMOST_ALL";
const std::string CALL_EXPR_FUNC_CALL = "CALL_EXPR_FUNC_CALL";

namespace CodeAnalysis {
ASTParser::ASTParser(int argc, const char **argv) :
    common_options_parser_(clang::tooling::CommonOptionsParser::create(argc, argv, llvm::cl::getGeneralCategory())),
    stream_string_buffer_(diagnostic_message_buffer_)
{
  LOG("初始化AST匹配器");
  using namespace clang::ast_matchers;
  auto matcher = namedDecl(unless(anyOf(hasAncestor((functionDecl())),
                                        hasAncestor(enumDecl()),
                                        hasAncestor((recordDecl()))))).bind(NAMED_DECL_OUTERMOST_ALL);
  auto matcher2 = callExpr(callee(functionDecl())).bind(CALL_EXPR_FUNC_CALL);
  finder_.addMatcher(matcher, &handler_);
  finder_.addMatcher(matcher2, &handler_);
}

void ASTParser::initASTs(const std::vector<std::string> &files)
{
  //  const std::string &string = toString(common_options_parser_.takeError());
  handler_.clear();
  
  asts.clear();
  LOG("使用文件列表构建AST树");
  clang::tooling::ClangTool tool{common_options_parser_->getCompilations(), files};
  tool.setDiagnosticConsumer(new TextDiagnosticPrinter(stream_string_buffer_, new DiagnosticOptions));
  tool.buildASTs(asts);
  parseAST();
}

std::string ASTParser::syntaxCheck()
{
  return stream_string_buffer_.str().str();
}

void ASTParser::parseAST()
{
  //  finder_.addMatcher(namedDecl(functionDecl().bind(NAMED_DECL_FUNC)), &handler_);
  //匹配全局变量
  //  auto matcher_global = varDecl(hasGlobalStorage(),
  //                                unless(anyOf(hasAncestor((functionDecl())), hasAncestor((recordDecl()))))).bind(
  //      NAMED_DECL_VAR_GLOBAL);
  //  //匹配其他声明
  //  auto matcher_record = recordDecl
  LOG("进行AST查找");
  for (const auto &ast: asts) {
    LOG("查找文件:{}", ast->getMainFileName().data());
    finder_.matchAST(ast->getASTContext());
    //    const StringRef &string1 = ast->getASTFileName();
    //    const StringRef &string2 = ast->getMainFileName();
    //    const StringRef &string3 = ast->getOriginalSourceFileName();
  }
}

std::string MatchHandler::getTypeAsString(const clang::NamedDecl *named_decl)
{
  switch (named_decl->getKind()) {
    case clang::Decl::Var:
      return dyn_cast<VarDecl>(named_decl)->getType().getAsString();
    case clang::Decl::Function:
      return dyn_cast<FunctionDecl>(named_decl)->getReturnType().getAsString();
    case clang::Decl::Record:
      return dyn_cast<RecordDecl>(named_decl)->getTypeForDecl()->getTypeClassName();
    case clang::Decl::Field:
      return dyn_cast<FieldDecl>(named_decl)->getType().getAsString();
    case clang::Decl::Enum:
      return dyn_cast<EnumDecl>(named_decl)->getTypeForDecl()->getTypeClassName();
    case clang::Decl::EnumConstant:
      return dyn_cast<EnumConstantDecl>(named_decl)->getType().getAsString();
    default:
      return {};
  }
}

void MatchHandler::run(const ast_matchers::MatchFinder::MatchResult &result)
{
  auto manager = result.SourceManager;
  if (auto decl = result.Nodes.getNodeAs<NamedDecl>(NAMED_DECL_OUTERMOST_ALL)) {
    if (!manager->isWrittenInMainFile(decl->getLocation())) {
      return;
    }
    std::filesystem::path p(manager->getFilename(decl->getLocation()).str());
    //    LOG("向map中添加文件:{}", std::filesystem::absolute(p).string());
    map_file_decl_[std::filesystem::absolute(p).string()].push_back(decl);
  }
  if (auto call = result.Nodes.getNodeAs<CallExpr>(CALL_EXPR_FUNC_CALL)) {
    if (!manager->isWrittenInMainFile(call->getBeginLoc())) {
      return;
    }
    std::filesystem::path p(manager->getFilename(call->getBeginLoc()).str());
    //    LOG("向map中添加文件:{}", std::filesystem::absolute(p).string());
    map_file_call_[std::filesystem::absolute(p).string()].push_back(call);
  }
}

std::map<const NamedDecl *, std::vector<const NamedDecl *>> MatchHandler::extractFunctionsAndVars(std::string filepath)
{
  using namespace std::filesystem;
  LOG("提取变量和函数，文件路径:{}", filepath);
  try {
    filepath = absolute(path(filepath)).string();
  } catch (const std::exception &ec) {
    LOG("捕获错误:{}", ec.what());
  }
  LOG("提取变量和函数，转换后的文件路径:{}", filepath);
  std::map<const NamedDecl *, std::vector<const NamedDecl *>> map_decl_decls;
  if (!map_file_decl_.contains(filepath)) {
    LOG("未找到文件");
    return map_decl_decls;
  }
  LOG("提取函数和变量，数量{}", map_file_decl_[filepath].size());
  for (const auto &item: map_file_decl_[filepath]) {
    map_decl_decls[item] = std::vector<const NamedDecl *>();
    switch (item->getKind()) {
      case clang::Decl::Function: {
        const auto *func_decl = dyn_cast<FunctionDecl>(item);
        //内部的声明
        for (const auto inner_decl: func_decl->decls()) {
          if (auto inner_decl_var = dyn_cast<NamedDecl>(inner_decl)) {
            map_decl_decls[item].push_back(inner_decl_var);
            //            LOG("变量:{},类型:{}", inner_decl_var->getNameAsString(), inner_decl_var->getDeclKindName());
          }
        }
      }
        break;
      case clang::Decl::Record: {
        const auto *record_decl = dyn_cast<RecordDecl>(item);
        //内部的声明
        for (const auto inner_decl: record_decl->decls()) {
          if (auto inner_decl_var = dyn_cast<NamedDecl>(inner_decl)) {
            map_decl_decls[item].push_back(inner_decl_var);
            //            LOG("变量名:{},类型:{}", inner_decl_var->getNameAsString(), inner_decl_var->getDeclKindName());
          }
        }
        break;
      }
      case clang::Decl::Enum: {
        const auto *enum_decl = dyn_cast<EnumDecl>(item);
        //内部的声明
        for (const auto inner_decl: enum_decl->decls()) {
          if (auto inner_decl_var = dyn_cast<NamedDecl>(inner_decl)) {
            map_decl_decls[item].push_back(inner_decl_var);
            //            LOG("变量名:{},类型:{}", inner_decl_var->getNameAsString(), inner_decl_var->getDeclKindName());
          }
        }
        break;
      }
      default:
        break;
    }
  }
  return map_decl_decls;
}

std::vector<const clang::NamedDecl *> MatchHandler::decls(std::string filepath)
{
  using namespace std::filesystem;
  filepath = absolute(path(filepath)).string();
  return map_file_decl_[filepath];
}

std::vector<const clang::CallExpr *> MatchHandler::callees(std::string filepath)
{
  using namespace std::filesystem;
  filepath = absolute(path(filepath)).string();
  return map_file_call_[filepath];
}

uint32_t MatchHandler::getLineNumber(const clang::NamedDecl *named_decl)
{
  return named_decl->getASTContext().getSourceManager().getSpellingLineNumber(named_decl->getLocation());
}

uint32_t MatchHandler::getColumnNumber(const clang::NamedDecl *named_decl)
{
  return named_decl->getASTContext().getSourceManager().getSpellingColumnNumber(named_decl->getLocation());
}

uint32_t MatchHandler::getLineNumber(const clang::CallExpr *callee)
{
  return callee->getDirectCallee()->getASTContext().getSourceManager().getSpellingLineNumber(callee->getBeginLoc());
}

//std::vector<const VarDecl *> MatchHandler::getVarDeclsByFuncName(const std::string &file, const std::string &func_name)
//{
//  LOG("函数名:{}", func_name);
//  std::vector<const VarDecl *> var_decls_;
//  auto iter = map_file_decl_[file].begin();
//  for (; iter != map_file_decl_[file].end(); ++iter) {
//    auto temp_func_name = (*iter)->getNameAsString();
//
//    if ((*iter)->isThisDeclarationADefinition() && temp_func_name == func_name) {
//      (*iter)->dumpColor();
//      break;
//    }
//  }
//  if (iter == map_file_decl_[file].end()) {
//    LOG("文件内部未找到该函数");
//    return var_decls_;
//  }
//  //    assert(iter != map_file_decl_[file].end());
//  LOG("内部变量:");
//  for (const auto inner_decl: (*iter)->decls()) {
//    if (auto inner_decl_var = dyn_cast<VarDecl>(inner_decl)) {
//      var_decls_.push_back(inner_decl_var);
//      LOG("变量名:{},类型:{}", inner_decl_var->getNameAsString(), inner_decl_var->getType().getAsString());
//    }
//  }
//  return var_decls_;
//}
std::string MatchHandler::getName(const clang::CallExpr *callee)
{
  return callee->getDirectCallee()->getNameAsString();
}

std::string MatchHandler::getFileName(const clang::CallExpr *callee)
{
  return callee->getDirectCallee()->getASTContext().getSourceManager().getFilename(callee->getBeginLoc()).str();
}

void MatchHandler::clear()
{
  map_file_call_.clear();
  map_file_decl_.clear();
}
} // CodeAnalysis
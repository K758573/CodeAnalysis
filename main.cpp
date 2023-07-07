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

#include <memory>
#include <iostream>
#include "src/Log.h"

using namespace clang;
using namespace llvm::cl;
const std::string NAMED_DECL_VAR = "named_decl_var";
const std::string NAMED_DECL_FUNC = "named_decl_func";
namespace {

class MatchHandler :
    public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
  std::vector<const VarDecl *> var_decls_;
  std::vector<const FunctionDecl *> func_decls_;
  
  
  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override
  {
    auto manager = result.SourceManager;
    auto context = result.Context;
    if (auto name = result.Nodes.getNodeAs<VarDecl>(NAMED_DECL_VAR)) {
      if (!manager->isWrittenInMainFile(name->getLocation())) {
        return;
      }
      //      name->dumpColor();
      //      var_decls_.push_back(clang::VarDecl::Create(name->getASTContext(),name->getDeclContext(),));
//      name->dumpColor();
      var_decls_.push_back(name);
    }
    
    if (auto name = result.Nodes.getNodeAs<FunctionDecl>(NAMED_DECL_FUNC)) {
      if (!manager->isWrittenInMainFile(name->getLocation())) {
        return;
      }
//      name->dumpColor();
      func_decls_.push_back(name);
      //      func_decls_.push_back();
    }
    
  }
};

}

int main(int argc, const char **argv)
{
  auto op = clang::tooling::CommonOptionsParser::create(argc, argv, llvm::cl::getGeneralCategory());
  clang::tooling::ClangTool tool{op->getCompilations(), op->getSourcePathList()};
  std::vector<std::unique_ptr<ASTUnit>> asts;
  tool.buildASTs(asts);
  for (const auto &ast: asts) {
    LOG("AST FILE NAME : {}", ast->getMainFileName().str());
    
  }
  using namespace clang::ast_matchers;
  MatchFinder finder;
  MatchHandler handler;
  finder.addMatcher(namedDecl(varDecl().bind(NAMED_DECL_VAR)), &handler);
  finder.addMatcher(namedDecl(functionDecl().bind(NAMED_DECL_FUNC)), &handler);
  for(const auto& ast:asts){
    finder.matchAST(ast->getASTContext());
  }
  for (const auto &item: handler.func_decls_) {
    item->dumpColor();
  }
  
  
  return 0;
}
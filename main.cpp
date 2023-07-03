// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"
#include "CodeAnalysis.h"
using namespace clang::tooling;
using namespace llvm;

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <clang/AST/ASTContext.h>

using namespace clang;
using namespace clang::ast_matchers;

StatementMatcher loopMatcher = forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(hasInitializer(integerLiteral(equals(0))))
                                                                              .bind("initVarName")))),
                                       hasIncrement(unaryOperator(hasOperatorName("++"),
                                                                  hasUnaryOperand(declRefExpr(to(varDecl(hasType(
                                                                      isInteger())).bind("incVarName")))))),
                                       hasCondition(binaryOperation(hasOperatorName("<"),
                                                                    hasLHS(ignoringParenImpCasts(declRefExpr(to(varDecl(
                                                                        hasType(isInteger())).bind("condVarName"))))),
                                                                    hasRHS(expr(hasType(isInteger())))))).bind("forLoop");
static bool areSameVariable(const ValueDecl *First, const ValueDecl *Second) {
  return First && Second &&
         First->getCanonicalDecl() == Second->getCanonicalDecl();
}
class LoopPrinter :
    public MatchFinder::MatchCallback
{
public :
  virtual void run(const MatchFinder::MatchResult &Result)
  {
    auto context = Result.Context;
    auto fs = Result.Nodes.getNodeAs<ForStmt>("forLoop");
    if (!fs || !context->getSourceManager().isWrittenInMainFile(fs->getForLoc())) {
      return ;
    }
    auto incVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
    auto condVar = Result.Nodes.getNodeAs<VarDecl>("condVarName");
    auto initVar = Result.Nodes.getNodeAs<VarDecl>("initVarName");
    if (!areSameVariable(incVar, condVar) || !areSameVariable(incVar, initVar)) {
      return;
    }
//    incVar->getLocation().printToString(SourceManager());
    LOG("incVar={}\ncondVar={}\ninitVar={}", incVar->getNameAsString(),condVar->getNameAsString(),initVar->getNameAsString());
    LOG("{}","");
  }
};

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char **argv)
{
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  LoopPrinter lp;
  MatchFinder finder;
  finder.addMatcher(loopMatcher, &lp);
  return Tool.run(newFrontendActionFactory(&finder).get());
}
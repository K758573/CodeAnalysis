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
std::vector<const CallExpr*> result;

static bool areSameVariable(const ValueDecl *First, const ValueDecl *Second)
{
  return First && Second && First->getCanonicalDecl() == Second->getCanonicalDecl();
}

class LoopPrinter :
    public MatchFinder::MatchCallback
{
public :
  void run(const MatchFinder::MatchResult &Result) override
  {
    auto context = Result.Context;
    auto fs = Result.Nodes.getNodeAs<ForStmt>("forLoop");
    if (!fs || !context->getSourceManager().isWrittenInMainFile(fs->getForLoc())) {
      return;
    }
    auto incVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
    auto condVar = Result.Nodes.getNodeAs<VarDecl>("condVarName");
    auto initVar = Result.Nodes.getNodeAs<VarDecl>("initVarName");
    if (!areSameVariable(incVar, condVar) || !areSameVariable(incVar, initVar)) {
      return;
    }
    auto begin_loc = context->getFullLoc(incVar->getBeginLoc());
    auto end_loc = context->getFullLoc(incVar->getEndLoc());
    
    //    incVar->getLocation().printToString(SourceManager());
    LOG("incVar={}\ncondVar={}\ninitVar={}",
        incVar->getNameAsString(),
        condVar->getNameAsString(),
        initVar->getNameAsString());
    LOG("incVar={}\ncondVar={}\ninitVar={}",
        incVar->getLocation().printToString(context->getSourceManager()),
        condVar->getLocation().printToString(context->getSourceManager()),
        initVar->getLocation().printToString(context->getSourceManager()));
    LOG("begin_loc=({},{})\nend_loc=({},{})",
        begin_loc.getSpellingLineNumber(),
        begin_loc.getColumnNumber(),
        end_loc.getSpellingLineNumber(),
        end_loc.getColumnNumber());
  }
};
//decl expr stmt
class FunctionPrinter :
    public MatchFinder::MatchCallback
{
public:
  void run(const MatchFinder::MatchResult &Result) override
  {
    auto context = Result.Context;
    auto ce = Result.Nodes.getNodeAs<Stmt>("functions");
    if (!ce||!context->getSourceManager().isWrittenInMainFile(ce->getBeginLoc())) {
      return;
    }
    ce->dumpColor();
//    auto full_location = context->getFullLoc(ce->getBeginLoc());
//    if (full_location.isValid()) {
//      llvm::outs() << "Found call at " << full_location.getSpellingLineNumber() << ":"
//                   << full_location.getSpellingColumnNumber() << "\n";
//    }
//    if (ce->getType().getTypePtrOrNull()) {
//      auto func= ce->getDirectCallee();
//      llvm::outs() << "函数名=" << func->getNameAsString();
//    }
//    ce->dump();
  }
};

int main(int argc, const char **argv)
{
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, cl::getGeneralCategory());
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  LoopPrinter lp;
  FunctionPrinter fp;
  MatchFinder finder;
  
  StatementMatcher loopMatcher = forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(hasInitializer(integerLiteral(equals(
                                             0)))).bind("initVarName")))),
                                         hasIncrement(unaryOperator(hasOperatorName("++"),
                                                                    hasUnaryOperand(declRefExpr(to(varDecl(hasType(
                                                                        isInteger())).bind("incVarName")))))),
                                         hasCondition(binaryOperation(hasOperatorName("<"),
                                                                      hasLHS(ignoringParenImpCasts(declRefExpr(to(
                                                                          varDecl(hasType(isInteger())).bind(
                                                                              "condVarName"))))),
                                                                      hasRHS(expr(hasType(isInteger())))))).bind(
      "forLoop");
  StatementMatcher function_call = callExpr(callee(functionDecl())).bind("functions");
  finder.addMatcher(loopMatcher, &lp);
  finder.addMatcher(function_call, &fp);
  int ret= Tool.run(newFrontendActionFactory(&finder).get());
//  runToolOnCode(std::make_unique<clang::TemplightDumpAction>(), "struct x{\n"
//                                                             "  int a;\n"
//                                                             "  float b;\n"
//                                                             "};");
  return ret;
}
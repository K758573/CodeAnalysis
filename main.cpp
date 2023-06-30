#include <iostream>
#include <clang-c/Index.h>
#include "src/Log.h"

int main()
{
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(index,
                                                      "example.c",
                                                      nullptr,
                                                      0,
                                                      nullptr,
                                                      0,
                                                      CXTranslationUnit_None);
  if (unit == nullptr) {
    LOG("无法解析");
    exit(-1);
  }
  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData client_data) {
    if (clang_getCursorKind(c) == CXCursor_CallExpr) {
      auto cursor = clang_getCursorSpelling(c);
      auto kind = clang_getCursorKindSpelling(clang_getCursorKind(c));
      LOG("spell: cursor={},kind={}", clang_getCString(cursor), clang_getCString(kind));
      clang_disposeString(cursor);
      clang_disposeString(kind);
    }
    return CXChildVisit_Recurse;
  }, nullptr);
  auto ret = clang_getCursorLocation(cursor);
  LOG("location:{}", ret.int_data);
  LOG("location:{}", ret.ptr_data[0]);
  LOG("location:{}", ret.ptr_data[1]);
  
  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return 0;
}

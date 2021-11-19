// coding: utf-8
// author: 18308013 ChenJiahao
#ifndef MYSEMANTICS_H
#define MYSEMANTICS_H
#include "myStructure.h"
#include "myLexer.h"
#include "myUtils.h"

// 声明变量 #V
// 声明常量 #C
// 临时变量 #TV
// 临时常量 #TC
// 函数 #F
// 标签 #L
# define SYMBOL_NEWNAME_COUNT_NUM 6

void updateScanIndex(token* tp);
void fprintSymbolTable(Symbol* header, int space, FILE* fp);
Symbol* buildNewSymbol(Symbol* latedPointer, Bool isChild);
Bool isStringEqual(const char* str1, const char* str2);
Bool isTokenEqual(const token* tp1, const token* tp2);
Bool isSymbolExist(const Symbol* latedPointer, const token* tp, SymbolType symType);
const Symbol* findSymbol(const Symbol* latedPointer, const char* name, SymbolType symType);
ValueType getValueType(TokenType tt);
Bool isNumInt(char *str);
Symbol* checkProgram(TreeNode* root, Symbol* latedSymPointer, FILE* fp);
Symbol* checkFunctionDecl(TreeNode* root, Symbol* latedSymPointer, FILE* fp, Bool* isHasReturn);
Symbol* checkBlock(TreeNode* root, Symbol* latedSymPointer, FILE* fp, Bool* isHasReturn);
Symbol* checkStatement(TreeNode* root, Symbol* latedSymPointer, FILE* fp, Bool* isHasReturn);
Symbol* checkBoolMultiExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Symbol* checkBoolMidExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Symbol* checkBoolExpression(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Symbol* checkExpression(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Symbol* checkMultiplicativeExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Symbol* checkPrimaryExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Symbol* checkAssignExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp);
Bool outputCompareCode(char** requiredName, char* name1, char* name2, token* tp, FILE* fp);
Bool outputTypeTransformCode(char** requiredName, char* name, ValueType valType, ValueType requiredType, FILE* fp);
Bool outputBinaryOperaterCode(char** requiredName, ValueType* newType, char* name1, ValueType valType1, char* name2, ValueType valType2, token* tp, FILE* fp);
Bool outputNotCode(char** requiredName, char* name, ValueType type, FILE* fp);
Bool outputMinusCode(char** requiredName, char* name, FILE* fp);
Bool buildNewStmtValue(char** requiredName);
Bool buildNewTempValue(char** requiredName);
Bool buildNewFuncName(char** requiredName);
Bool buildNewLabel(char** requiredName);

# endif
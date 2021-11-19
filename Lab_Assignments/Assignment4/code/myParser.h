// coding: utf-8
// author: 18308013 ChenJiahao
#ifndef MYPARSER_H
#define MYPARSER_H
#include "myLexer.h"
#include "myUtils.h"
#include "myStructure.h"

TreeNode* buildTreeNode(Bool isFactor);// 构建语法树节点
void freeTreeNode(TreeNode* root);// 释放语法树
int getParserTreeMaxDepth(TreeNode* root);// 获取语法树深度
void printParserTree(TreeNode* root, int depth, Bool* PrintFlagPtr);// 显示语法树
void fprintParserTree(TreeNode* root, int depth, Bool* PrintFlagPtr, FILE* fp);// 显示语法树
void addChildNode(TreeNode* fp, TreeNode* cp);// 添加子节点
TreeNode* match(token** tpl, int* IndexPtr, int MaxTokenNum, TokenType type, Bool isThrow);
TreeNode* analyseProgram(token** tpl, int MaxTokenNum);
TreeNode* analyseFunctionDecl(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseType(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseFormalParams(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseFormalParam(token** tpl, int* IndexPtr, int MaxTokenNum);
Bool isFormalParam(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseBlock(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseStatement(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseLocalVarDecl(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseAssignStmt(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseReturnStmt(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseIfStmt(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseWhileStmt(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseWriteStmt(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseReadStmt(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseExpression(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseMultiplicativeExpr(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analysePrimaryExpr(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseAssignExpr(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseBoolMultiExpr(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseBoolMidExpr(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseBoolExpression(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseActualParams(token** tpl, int* IndexPtr, int MaxTokenNum);

#endif
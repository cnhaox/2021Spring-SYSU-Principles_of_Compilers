// coding: utf-8
// author: 18308013 ChenJiahao
#ifndef MYPARSER_H
#define MYPARSER_H
#include "myLexer.h"
#include "myUtils.h"

// 节点类型
typedef enum NodeType
{
    NT_PROGRAM,
    NT_FUNCTION_DECL,
    NT_TYPE,
    NT_FORMAL_PARAMS,
    NT_FORMAL_PARAM,
    NT_BLOCK,
    NT_STATEMENT,
    NT_LOCAL_VAR_DECL,
    NT_ASSIGN_STMT,
    NT_RETURN_STMT,
    NT_IF_STMT,
    NT_WHILE_STMT,
    NT_WRITE_STMT,
    NT_READ_STMT,
    NT_EXPRESSION,
    NT_MULTIPLICATIVE_EXPR,
    NT_PRIMARY_EXPR,
    NT_ASSIGN_EXPR,
    NT_BOOL_MULTI_EXPR,
    NT_BOOL_EXPRESSION,
    NT_ACTUAL_PARAMS,
    NT_FACTOR,
} NodeType;

// 节点
typedef struct TreeNode
{
    NodeType nodetype;// 节点类型
    struct TreeNode** ChildPtrList;// 子节点指针数组的指针
    int ChildCurrentIndex;// 当前子结点数量
    int ChildMaxNum;// 子结点指针数组支持的最大数量
    token* tp;// 叶子节点对应的TOKEN

} TreeNode;



TreeNode* buildTreeNode(Bool isFactor);// 构建语法树
void freeTreeNode(TreeNode* root);// 释放语法树
int getParserTreeMaxDepth(TreeNode* root);// 获取语法树深度
void printParserTree(TreeNode* root, int depth, Bool* PrintFlagPtr);// 显示语法树
void throwParserError(token* tp, const char* cp);// 显示错误
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
TreeNode* analyseBoolExpression(token** tpl, int* IndexPtr, int MaxTokenNum);
TreeNode* analyseActualParams(token** tpl, int* IndexPtr, int MaxTokenNum);

#endif
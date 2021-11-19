// coding: utf-8
// author: 18308013 ChenJiahao
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myLexer.h"
#include "myParser.h"

#define CHILDMAXNUM 10

const char* NodeTypeNames[] = {
    "Program",
    "FuntionDecl",
    "Type",
    "FormalParams",
    "FormalParam",
    "Block",
    "Statement",
    "LocalVarDecl",
    "AssignStmt",
    "ReturnStmt",
    "IfStmt",
    "WhileStmt",
    "WriteStmt",
    "ReadStmt",
    "Expression",
    "MultiplicativeExpr",
    "PrimaryExpr",
    "AssignExpr",
    "BoolMultiExpr",
    "BoolMidExpr",
    "BoolExpression",
    "ActualParams",
    "Factor"
};

// 构建语法树
TreeNode* buildTreeNode(Bool isFactor)
{
    TreeNode* np = (TreeNode*)calloc(1, sizeof(TreeNode));
    np->tp = NULL;
    np->ChildCurrentIndex = 0;
    np->ChildMaxNum = 0;
    if (isFactor == False)
    {
        np->ChildMaxNum = CHILDMAXNUM;
        np->ChildPtrList = (TreeNode**)calloc(np->ChildMaxNum, sizeof(TreeNode*));
    }
    return np;
}

// 释放语法树
void freeTreeNode(TreeNode* root)
{
    for (int i = 0; i < root->ChildCurrentIndex; i++)
        freeTreeNode(root->ChildPtrList[i]);
    free(root);
    return;
}

// 获取语法树深度
int getParserTreeMaxDepth(TreeNode* root)
{
    if (root == NULL)
        return 0;
    if (root->nodetype == NT_FACTOR)
        return 1;
    else
    {
        int maxDepth = 0;
        int temp = 0;
        for (int i = 0; i < root->ChildCurrentIndex; i++)
        {
            temp = getParserTreeMaxDepth(root->ChildPtrList[i]);
            if (maxDepth < temp)
                maxDepth = temp;
        }
        return maxDepth + 1;
    }
}

// 显示语法树
void printParserTree(TreeNode* root, int depth, Bool* PrintFlagPtr)
{
    if (root == NULL)
        return;
    for (int i = 1; i < depth - 1; i++)
    {
        if (PrintFlagPtr[i - 1] == True)
            printf("|   ");
        else
            printf("    ");
    }
    if (depth > 1)
        printf("|__ ");
    printf("%s", NodeTypeNames[root->nodetype]);
    if (root->nodetype == NT_FACTOR)
        printf(": (%s, %s)", getTokenName(root->tp->type), root->tp->str);
    printf("\n");
    for (int i = 0; i < root->ChildCurrentIndex; i++)
    {
        if (i < root->ChildCurrentIndex - 1)
            PrintFlagPtr[depth - 1] = True;
        else
            PrintFlagPtr[depth - 1] = False;
        printParserTree(root->ChildPtrList[i], depth + 1, PrintFlagPtr);

    }
}

void fprintParserTree(TreeNode* root, int depth, Bool* PrintFlagPtr, FILE* fp)
{
    if (root == NULL)
        return;
    for (int i = 1; i < depth - 1; i++)
    {
        if (PrintFlagPtr[i - 1] == True)
            fprintf(fp,"|   ");
        else
            fprintf(fp,"    ");
    }
    if (depth > 1)
        fprintf(fp,"|__ ");
    fprintf(fp,"%s", NodeTypeNames[root->nodetype]);
    if (root->nodetype == NT_FACTOR)
        fprintf(fp,": (%s, %s)", getTokenName(root->tp->type), root->tp->str);
    fprintf(fp,"\n");
    for (int i = 0; i < root->ChildCurrentIndex; i++)
    {
        if (i < root->ChildCurrentIndex - 1)
            PrintFlagPtr[depth - 1] = True;
        else
            PrintFlagPtr[depth - 1] = False;
        fprintParserTree(root->ChildPtrList[i], depth + 1, PrintFlagPtr,fp);

    }
}
// 添加子节点
void addChildNode(TreeNode* fp, TreeNode* cp)
{
    // 空间不够的话扩容
    if (fp->ChildCurrentIndex >= fp->ChildMaxNum)
    {
        fp->ChildMaxNum += CHILDMAXNUM;
        fp->ChildPtrList = (TreeNode**)realloc(fp->ChildPtrList, fp->ChildMaxNum * sizeof(TreeNode*));
        for (int i = fp->ChildCurrentIndex; i < fp->ChildMaxNum; i++)
            fp->ChildPtrList[i] = NULL;
    }
    // 添加
    fp->ChildPtrList[fp->ChildCurrentIndex++] = cp;
}

TreeNode* match(token** tpl, int* IndexPtr, int MaxTokenNum, TokenType type, Bool isThrow)
{
    TreeNode* np = NULL;
    // 若匹配对应类型
    if (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == type)
    {
        // 创建新节点并赋值
        np = buildTreeNode(True);
        np->nodetype = NT_FACTOR;
        np->tp = tpl[*IndexPtr];
        // 偏移+1
        (*IndexPtr)++;
    }
    // 不匹配，报错
    // isThrow: 是否抛出异常语句
    if (isThrow && np == NULL)
    {
        char output[MAXMESLEN];
        if (*IndexPtr >= MaxTokenNum)
        {
            snprintf(output, MAXMESLEN, "(%s,%s) Expect a %s.", getTokenName(tpl[*IndexPtr - 1]->type), tpl[*IndexPtr - 1]->str, getTokenName(type));
            throwParserError(tpl[*IndexPtr - 1], output);
        }
        else
        {
            snprintf(output, MAXMESLEN, "(%s,%s) Expect a %s.", getTokenName(tpl[*IndexPtr]->type), tpl[*IndexPtr]->str, getTokenName(type));
            throwParserError(tpl[*IndexPtr], output);
        }
    }
    return np;
}

TreeNode* analyseProgram(token** tpl, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    fp->nodetype = NT_PROGRAM;
    TreeNode* cp = NULL;

    int TokenIndex = 0;
    while (TokenIndex < MaxTokenNum && tpl[TokenIndex]->type != FINISH)
    {
        cp = analyseFunctionDecl(tpl, &TokenIndex, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (TokenIndex < MaxTokenNum)
                throwParserError(tpl[TokenIndex], "EunctionDecl doesn't finish.");
            else
                throwParserError(tpl[TokenIndex - 1], "EunctionDecl doesn't finish.");
            return NULL;
        }
    }
    if (TokenIndex >= MaxTokenNum || tpl[TokenIndex]->type != FINISH)
    {
        if (TokenIndex < MaxTokenNum)
            throwParserError(tpl[TokenIndex], "Program doesn't finish.");
        else
            throwParserError(tpl[TokenIndex - 1], "Program doesn't finish.");
        return NULL;
    }

    return fp;
}

TreeNode* analyseFunctionDecl(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_FUNCTION_DECL;
    TreeNode* cp = NULL;

    cp = analyseType(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Type doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Type doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, MAIN, False);
    if (cp != NULL)
        addChildNode(fp, cp);

    cp = match(tpl, IndexPtr, MaxTokenNum, ID, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, LP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseFormalParams(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "FormalParams doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "FormalParams doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseBlock(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Block doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Block doesn't finish.");
        return NULL;
    }

    return fp;
}

TreeNode* analyseType(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_TYPE;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, INT, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, REAL, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, BOOL, False);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        char output[MAXMESLEN];
        if (*IndexPtr < MaxTokenNum)
        {
            snprintf(output, MAXMESLEN, "(%s,%s) Expect a INT/REAL/BOOL.", getTokenName(tpl[*IndexPtr]->type), tpl[*IndexPtr]->str);
            throwParserError(tpl[*IndexPtr], output);
        }
        else
        {
            snprintf(output, MAXMESLEN, "(%s,%s) Expect a INT/REAL/BOOL.", getTokenName(tpl[*IndexPtr-1]->type), tpl[*IndexPtr-1]->str);
            throwParserError(tpl[*IndexPtr - 1], output);
        }
        return NULL;
    }

    return fp;
}

TreeNode* analyseFormalParams(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_FORMAL_PARAMS;
    TreeNode* cp;

    if (isFormalParam(tpl, IndexPtr, MaxTokenNum))
    {
        cp = analyseFormalParam(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "FormalParam doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "FormalParam doesn't finish.");
            return NULL;
        }

        while (tpl[*IndexPtr]->type == COMMA)
        {
            cp = match(tpl, IndexPtr, MaxTokenNum, COMMA, True);
            if (cp != NULL)
                addChildNode(fp, cp);
            else
                return NULL;

            cp = analyseFormalParam(tpl, IndexPtr, MaxTokenNum);
            if (cp != NULL)
                addChildNode(fp, cp);
            else
            {
                if (*IndexPtr < MaxTokenNum)
                    throwParserError(tpl[*IndexPtr], "FormalParam doesn't finish.");
                else
                    throwParserError(tpl[*IndexPtr - 1], "FormalParam doesn't finish.");
                return NULL;
            }
        }
    }

    return fp;
}

TreeNode* analyseFormalParam(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_FORMAL_PARAM;
    TreeNode* cp = NULL;

    cp = analyseType(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Type doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Type doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, ID, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

Bool isFormalParam(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    int OriginIndex = *IndexPtr;
    Bool flag = True;
    if (*IndexPtr < MaxTokenNum)
    {
        TokenType temp = tpl[*IndexPtr]->type;
        if (temp == INT || temp == REAL || temp == BOOL)
            (*IndexPtr)++;
        else
            flag = False;
    }
    else
        flag = False;
    if (flag && *IndexPtr < MaxTokenNum)
    {
        if (tpl[*IndexPtr]->type == ID)
            flag = True;
    }
    else
        flag = False;

    *IndexPtr = OriginIndex;
    return flag;
}

TreeNode* analyseBlock(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_BLOCK;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, BEGIN, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    while (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type != END)
    {
        cp = analyseStatement(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "Statement doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "Statement doesn't finish.");
            return NULL;
        }
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, END, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

TreeNode* analyseStatement(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_STATEMENT;
    TreeNode* cp;

    if (*IndexPtr >= MaxTokenNum)
        return NULL;

    const char* output;

    switch (tpl[*IndexPtr]->type)
    {
    case BEGIN:
        cp = analyseBlock(tpl, IndexPtr, MaxTokenNum);
        output = "Block doesn't finish.";
        break;
    case INT:
    case REAL:
    case BOOL:
        cp = analyseLocalVarDecl(tpl, IndexPtr, MaxTokenNum);
        output = "LocalVarDecl doesn't finish.";
        break;
    case ID:
        cp = analyseAssignStmt(tpl, IndexPtr, MaxTokenNum);
        output = "AssignStmt doesn't finish.";
        break;
    case RETURN:
        cp = analyseReturnStmt(tpl, IndexPtr, MaxTokenNum);
        output = "ReturnStmt doesn't finish.";
        break;
    case IF:
        cp = analyseIfStmt(tpl, IndexPtr, MaxTokenNum);
        output = "IfStmt doesn't finish.";
        break;
    case WHILE:
        cp = analyseWhileStmt(tpl, IndexPtr, MaxTokenNum);
        output = "WhileStmt doesn't finish.";
        break;
    case WRITE:
        cp = analyseWriteStmt(tpl, IndexPtr, MaxTokenNum);
        output = "WriteStmt doesn't finish.";
        break;
    case READ:
        cp = analyseReadStmt(tpl, IndexPtr, MaxTokenNum);
        output = "ReadStmt doesn't finish.";
        break;
    default:
        cp = NULL;
        output = "Unknown Error.";
        break;
    }
    if (cp == NULL)
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], output);
        else
            throwParserError(tpl[*IndexPtr - 1], output);
        return NULL;
    }
    else
        addChildNode(fp, cp);

    return fp;
}

TreeNode* analyseLocalVarDecl(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_LOCAL_VAR_DECL;
    TreeNode* cp = NULL;

    cp = analyseType(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Type doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Type doesn't finish.");
        return NULL;
    }

    cp = analyseAssignExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "AssignExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "AssignExpr doesn't finish.");
        return NULL;
    }

    while (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == COMMA)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, COMMA, False);
        addChildNode(fp, cp);
        
        cp = analyseAssignExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "AssignExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "AssignExpr doesn't finish.");
            return NULL;
        }
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, SEMICOLON, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

TreeNode* analyseAssignStmt(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_ASSIGN_STMT;
    TreeNode* cp = NULL;

    cp = analyseAssignExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "AssignExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "AssignExpr doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, SEMICOLON, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

TreeNode* analyseReturnStmt(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_RETURN_STMT;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, RETURN, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, SEMICOLON, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

TreeNode* analyseIfStmt(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_IF_STMT;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, IF, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, LP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseStatement(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Statement doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Statement doesn't finish.");
        return NULL;
    }

    if (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == ELSE)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, ELSE, True);
        addChildNode(fp, cp);

        cp = analyseStatement(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "Statement doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "Statement doesn't finish.");
            return NULL;
        }
    }

    return fp;
}

TreeNode* analyseWhileStmt(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_WHILE_STMT;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, WHILE, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, LP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseStatement(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Statement doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Statement doesn't finish.");
        return NULL;
    }

    return fp;
}

TreeNode* analyseWriteStmt(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_WRITE_STMT;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, WRITE, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, LP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
        return NULL;
    }

    cp = match(tpl, IndexPtr, MaxTokenNum, COMMA, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, STRING, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, SEMICOLON, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

TreeNode* analyseReadStmt(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_READ_STMT;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, READ, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, LP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, ID, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, COMMA, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, STRING, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, SEMICOLON, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    return fp;
}

TreeNode* analyseExpression(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_EXPRESSION;
    TreeNode* cp = NULL;

    cp = analyseMultiplicativeExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "MultiplicativeExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "MultiplicativeExpr doesn't finish.");
        return NULL;
    }

    while (*IndexPtr < MaxTokenNum && (tpl[*IndexPtr]->type == ADD || tpl[*IndexPtr]->type == SUB))
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, ADD, False);
        if (cp == NULL)
            cp = match(tpl, IndexPtr, MaxTokenNum, SUB, False);
        addChildNode(fp, cp);

        cp = analyseMultiplicativeExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "MultiplicativeExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "MultiplicativeExpr doesn't finish.");
            return NULL;
        }
    }

    return fp;
}

TreeNode* analyseMultiplicativeExpr(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_MULTIPLICATIVE_EXPR;
    TreeNode* cp = NULL;

    cp = analysePrimaryExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "PrimaryExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "PrimaryExpr doesn't finish.");
        return NULL;
    }

    while (*IndexPtr < MaxTokenNum && (tpl[*IndexPtr]->type == MUL || tpl[*IndexPtr]->type == DIV || tpl[*IndexPtr]->type == MOD))
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, MUL, False);
        if (cp == NULL)
            cp = match(tpl, IndexPtr, MaxTokenNum, DIV, False);
        if (cp == NULL)
            cp = match(tpl, IndexPtr, MaxTokenNum, MOD, False);
        addChildNode(fp, cp);

        cp = analysePrimaryExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "PrimaryExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "PrimaryExpr doesn't finish.");
            return NULL;
        }
    }

    return fp;
}

TreeNode* analysePrimaryExpr(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_PRIMARY_EXPR;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, NOT, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, SUB, False);
    if (cp != NULL)
        addChildNode(fp, cp);
    
    cp = NULL;
    cp = match(tpl, IndexPtr, MaxTokenNum, NUMBER, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, TRUE, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, FALSE, False);
    if (cp != NULL)
    {
        addChildNode(fp, cp);
        return fp;
    }
    else
        cp = match(tpl, IndexPtr, MaxTokenNum, ID, False);
    if (cp != NULL)
    {
        addChildNode(fp, cp);
        if (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == LP)
        {
            cp = match(tpl, IndexPtr, MaxTokenNum, LP, False);
            addChildNode(fp, cp);
            if (*IndexPtr >= MaxTokenNum || tpl[*IndexPtr]->type != RP)
            {
                cp = analyseActualParams(tpl, IndexPtr, MaxTokenNum);
                if (cp != NULL)
                    addChildNode(fp, cp);
                else
                {
                    if (*IndexPtr < MaxTokenNum)
                        throwParserError(tpl[*IndexPtr], "ActualParams doesn't finish.");
                    else
                        throwParserError(tpl[*IndexPtr - 1], "ActualParams doesn't finish.");
                    return NULL;
                }
            }
            cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
            if (cp != NULL)
                addChildNode(fp, cp);
            else
                return NULL;
        }
        return fp;
    }
    else
        cp = match(tpl, IndexPtr, MaxTokenNum, LP, False);

    if (cp == NULL)
    {
        if (*IndexPtr >= MaxTokenNum)
            throwParserError(tpl[*IndexPtr - 1], "PrimaryExpr matches none.");
        else
            throwParserError(tpl[*IndexPtr], "PrimaryExpr matches none.");
        return NULL;
    }
    else
    {
        addChildNode(fp, cp);
        cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
            return NULL;
        }

        cp = match(tpl, IndexPtr, MaxTokenNum, RP, True);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
            return NULL;
    }

    return fp;
}

TreeNode* analyseAssignExpr(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_ASSIGN_EXPR;
    TreeNode* cp = NULL;

    cp = match(tpl, IndexPtr, MaxTokenNum, ID, True);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
        return NULL;

    if (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == ASSIGN)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, ASSIGN, False);
        addChildNode(fp, cp);

        cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
            return NULL;
        }
    }
    return fp;
}

TreeNode* analyseBoolMultiExpr(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_BOOL_MULTI_EXPR;
    TreeNode* cp = NULL;

    cp = analyseBoolMidExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolMidExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolMidExpr doesn't finish.");
        return NULL;
    }

    while (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == AND)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, AND, False);
        addChildNode(fp, cp);

        cp = analyseBoolMidExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "BoolMidExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "BoolMidExpr doesn't finish.");
            return NULL;
        }
    }

    return fp;
}

TreeNode* analyseBoolMidExpr(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_BOOL_MID_EXPR;
    TreeNode* cp = NULL;

    cp = analyseBoolExpression(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolExpression doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolExpression doesn't finish.");
        return NULL;
    }

    while (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == OR)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, OR, False);
        addChildNode(fp, cp);

        cp = analyseBoolExpression(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "BoolExpression doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "BoolExpression doesn't finish.");
            return NULL;
        }
    }

    return fp;
}

TreeNode* analyseBoolExpression(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_BOOL_EXPRESSION;
    TreeNode* cp = NULL;
    /*
    if (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == NOT)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, NOT, False);
        addChildNode(fp, cp);

        cp = analyseExpression(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "Expression doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "Expression doesn't finish.");
            return NULL;
        }

        return fp;
    }*/
    cp = analyseExpression(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Expression doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Expression doesn't finish.");
        return NULL;
    }


    cp = match(tpl, IndexPtr, MaxTokenNum, EQUAL, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, UNEQUAL, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, LEQ, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, GEQ, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, LT, False);
    if (cp == NULL)
        cp = match(tpl, IndexPtr, MaxTokenNum, GT, False);
    if (cp == NULL)
        return fp;
    else
        addChildNode(fp, cp);

    cp = analyseExpression(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "Expression doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "Expression doesn't finish.");
        return NULL;
    }

    return fp;
}

TreeNode* analyseActualParams(token** tpl, int* IndexPtr, int MaxTokenNum)
{
    TreeNode* fp = buildTreeNode(False);
    int ChildIndex = 0;
    fp->nodetype = NT_ACTUAL_PARAMS;
    TreeNode* cp = NULL;

    cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
    if (cp != NULL)
        addChildNode(fp, cp);
    else
    {
        if (*IndexPtr < MaxTokenNum)
            throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
        else
            throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
        return NULL;
    }

    while (*IndexPtr < MaxTokenNum && tpl[*IndexPtr]->type == COMMA)
    {
        cp = match(tpl, IndexPtr, MaxTokenNum, COMMA, False);
        addChildNode(fp, cp);

        cp = analyseBoolMultiExpr(tpl, IndexPtr, MaxTokenNum);
        if (cp != NULL)
            addChildNode(fp, cp);
        else
        {
            if (*IndexPtr < MaxTokenNum)
                throwParserError(tpl[*IndexPtr], "BoolMultiExpr doesn't finish.");
            else
                throwParserError(tpl[*IndexPtr - 1], "BoolMultiExpr doesn't finish.");
            return NULL;
        }
    }

    return fp;
}


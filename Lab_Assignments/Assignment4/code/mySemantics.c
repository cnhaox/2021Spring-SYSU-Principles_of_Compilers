#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myUtils.h"
#include "myLexer.h"
#include "myParser.h"
#include "mySemantics.h"


int RowScanIndex = 0; // 当前行号
int ColScanIndex = 0; // 当前列号
int SymbolNum[SYMBOL_NEWNAME_COUNT_NUM] = {0}; // 三地址码命名计数

// 更新当前行号和列号
void updateScanIndex(token* tp)
{
    RowScanIndex = tp->RowIndex;
    ColScanIndex = tp->ColIndex;
}

const char* SymBolTypeNames[] = {
    "ST_FUNC",  // 函数
    "ST_VAR",   // 变量
    "ST_TABLE", // block符号表
    "ST_HEAD",  // 符号表头部，无意义
    "ST_ANY"    // 未知
};

const char* ValueTypeNames[] = {
    "VT_INT",
    "VT_REAL",
    "VT_BOOL",
    "VT_STRING",
    "VT_VOID",
    "VT_ANY"  // 未知
};

// 打印符号表
void fprintSymbolTable(Symbol* header, int space, FILE* fp)
{
    while(header!=NULL)
    {
        for (int i=0;i<space;i++)
            fprintf(fp, "        ");
        fprintf(fp, "%s %s ", SymBolTypeNames[header->symType], ValueTypeNames[header->valType]);
        if (header->symType == ST_FUNC || header->symType == ST_VAR)
        {
            fprintToken(header->tp, fp);
            fprintf(fp, " %s\n", header->newName);
        }
        else
            fprintf(fp, "NULL NULL\n");
        if (header->symType==ST_FUNC||header->symType==ST_TABLE)
            fprintSymbolTable(header->childSymPointer, space+1, fp);
        header = header->nextSymPointer;
    }
}

/*
    构建符号表新节点
    params: (Symbol*)latedPointer: 上一个节点; (Bool)isChild: 是否为子节点
    return: (Symbol*)latedSymPointer: 新构建的节点
*/
Symbol* buildNewSymbol(Symbol* latedPointer, Bool isChild)
{
    Symbol *latedSymPointer = (Symbol*)calloc(1, sizeof(Symbol));
    if (isChild)
    {
        latedSymPointer->parentSymPointer = latedPointer;
        latedSymPointer->prevSymPointer = NULL;
        latedSymPointer->symType = ST_HEAD;
    }
    else
    {
        latedSymPointer->parentSymPointer = NULL;
        latedSymPointer->prevSymPointer = latedPointer;
        latedSymPointer->symType = ST_ANY;
    }
    latedSymPointer->nextSymPointer = NULL;
    latedSymPointer->tp = NULL;
    latedSymPointer->childSymPointer = NULL;
    latedSymPointer->valType = VT_ANY;
    latedSymPointer->isVariable = False;
    latedSymPointer->address = -1;
    latedSymPointer->size = -1;
    latedSymPointer->newName = NULL;
    if (latedPointer!=NULL)
    {
        if (isChild)
            latedPointer->childSymPointer = latedSymPointer;
        else
            latedPointer->nextSymPointer = latedSymPointer;
    }
    return latedSymPointer;
}

/*
    判断字符串是否相等
    params: (char*)str1: 字符串1; (char*)str2: 字符串2
    return: (Bool)flag
*/
Bool isStringEqual(const char* str1, const char* str2) 
{
    if (str1==NULL&&str2==NULL)
        return True;
    else if (str1==NULL||str2==NULL)
        return False;
    int i = 0;
    Bool flag = True;
    while (flag)
    {
        if (str1[i]==str2[i])
        {
            if (str1[i]=='\0')
                break;
            else
                i++;
        }
        else
            flag = False;
    }
    return flag;
}

/*
    判断Token是否相等
    params: (token*)tp1: ; (token*)str2: 
    return: (Bool)flag
*/
Bool isTokenEqual(const token* tp1, const token* tp2)
{
    if (tp1==NULL&&tp2==NULL)
        return True;
    if (tp1==NULL||tp2==NULL)
        return False;
    if (tp1->type!=tp2->type)
        return False;
    if (isStringEqual(tp1->str, tp2->str))
        return True;
    else
        return False;
}

/*
    判断token是否在当前符号表中存在
    params: (Symbol*)latedPointer: 符号表最后一个节点; (token*)tp: token; (SymbolType)symType: token符号类型
    return: (Bool)flag
*/
Bool isSymbolExist(const Symbol* latedPointer, const token* tp, SymbolType symType)
{
    Symbol *p = latedPointer;
    Bool flag = False;
    while (p!=NULL&&flag==False)
    {
        if (isTokenEqual(p->tp, tp)&&(symType==ST_ANY||symType==p->symType))
            flag = True;
        p = p->prevSymPointer;
    }
    return flag;
}

/*
    查找在符号表和父符号表中查找name对应的符号项
    params: (Symbol*)latedPointer: 符号表最后一个节点; (const char*)name: ; (SymbolType)symType: token符号类型
    return: (Bool)flag
*/
const Symbol* findSymbol(const Symbol* latedPointer, const char* name, SymbolType symType)
{
    while (latedPointer!=NULL&&(latedPointer->prevSymPointer!=NULL||latedPointer->parentSymPointer!=NULL))
    {
        if (latedPointer->symType==symType)
        {
            if (isStringEqual(latedPointer->tp->str, name))
                return latedPointer;
        }
        if (latedPointer->symType==ST_HEAD)
            latedPointer = latedPointer->parentSymPointer;
        else
            latedPointer = latedPointer->prevSymPointer;
    }
    return NULL;
}

// TokenType类型->ValueType类型
ValueType getValueType(TokenType tt)
{
    switch (tt)
        {
        case INT:
            return VT_INT;
        case REAL:
            return VT_REAL;
        case BOOL:
            return VT_BOOL;
        case STRING:
            return VT_STRING;
        default:
            return VT_ANY;
        }
}

// 判断数字是否为int
Bool isNumInt(char *str)
{
    int i=0;
    Bool flag = True;
    while(flag&&str[i]!='\0')
    {
        if (str[i]=='.')
            flag = False;
        i++;
    }
    return flag;
}

Symbol* checkProgram(TreeNode* root, Symbol* latedSymPointer, FILE *fp)
{
    for (int i=0; i<root->ChildCurrentIndex; i++)
    {
        // 是否有返回语句
        Bool isHasReturn = False;
        latedSymPointer = checkFunctionDecl(root->ChildPtrList[i], latedSymPointer, fp, &isHasReturn);
        if (latedSymPointer==NULL)
            return NULL;
        if (isHasReturn == False)
        {
            throwError(RowScanIndex, ColScanIndex, "The function doesn't have return statement.");
            return NULL;
        }
    }
    return latedSymPointer;
}

Symbol* checkFunctionDecl(TreeNode* root, Symbol* latedSymPointer, FILE *fp, Bool* isHasReturn)
{
    // 构建函数名符号项
    latedSymPointer = buildNewSymbol(latedSymPointer, False);
    latedSymPointer->symType = ST_FUNC;
    latedSymPointer->newName = NULL;
    buildNewFuncName(&(latedSymPointer->newName));
    // 设置调用函数标志
    fprintf(fp, "Label %s\n", latedSymPointer->newName);
    // 创建子符号表
    Symbol* newSymPointer = buildNewSymbol(latedSymPointer, True);
    TreeNode* tempNode;
    for (int childIndex = 0; childIndex<root->ChildCurrentIndex; childIndex++)
    {
        tempNode = root->ChildPtrList[childIndex];
        if (tempNode->nodetype==NT_FACTOR)
        {
            updateScanIndex(tempNode->tp);
            // 存在MAIN，输出Label MAIN
            if (tempNode->tp->type==MAIN)
            fprintf(fp, "Label MAIN\n");
            else if (tempNode->tp->type==ID)
            {
                // 查看是否存在同名函数
                if (isSymbolExist(latedSymPointer, tempNode->tp, ST_FUNC))
                {
                    throwError(RowScanIndex, ColScanIndex, "A function with the same name already exists in this scope.");
                    return NULL;
                }
                latedSymPointer->tp = tempNode->tp;
            }
        }
        else if (tempNode->nodetype==NT_TYPE)
            latedSymPointer->valType = getValueType(tempNode->ChildPtrList[0]->tp->type);
        else if (tempNode->nodetype==NT_FORMAL_PARAMS)
        {
            // 获取参数
            for (int i=0; i<= tempNode->ChildCurrentIndex - 1; i+=2)
            {
                newSymPointer = buildNewSymbol(newSymPointer, False);
                newSymPointer->symType = ST_VAR;
                newSymPointer->valType = getValueType(tempNode->ChildPtrList[i]->ChildPtrList[0]->ChildPtrList[0]->tp->type);
                token* idp = tempNode->ChildPtrList[i]->ChildPtrList[1]->tp;
                updateScanIndex(idp);
                // 判断参数之间是否重名
                if (isSymbolExist(newSymPointer, idp, ST_VAR)==True)
                {
                    throwError(RowScanIndex, ColScanIndex, "The parameter with the same name exists.");
                    return NULL;
                }
                newSymPointer->tp = idp;
                newSymPointer->isVariable = True;
                newSymPointer->isParam = True;
                newSymPointer->newName = NULL;
                buildNewStmtValue(&(newSymPointer->newName));
            }
            // 由于是弹栈操作，故要倒着显示
            for (Symbol* tempp = newSymPointer; tempp->symType!=ST_HEAD; tempp=tempp->prevSymPointer)
                fprintf(fp, "    get %s\n", tempp->newName);
        }
        else if (tempNode->nodetype==NT_BLOCK)
        {
            newSymPointer = checkBlock(tempNode, newSymPointer, fp, isHasReturn);
            if (newSymPointer==NULL)
                return NULL;
        }
        else
        {
            throwError(RowScanIndex, ColScanIndex, "Unknown error.@1");
            return NULL;
        }
    }
    return latedSymPointer;
}


Symbol* checkBlock(TreeNode* root, Symbol* latedSymPointer, FILE* fp, Bool* isHasReturn)
{
    for (int childIndex = 0; childIndex < root->ChildCurrentIndex; childIndex++)
    {
        if (root->ChildPtrList[childIndex]->nodetype == NT_FACTOR)
            updateScanIndex(root->ChildPtrList[childIndex]->tp);
        else
        {
            latedSymPointer = checkStatement(root->ChildPtrList[childIndex], latedSymPointer, fp, isHasReturn);
            if (latedSymPointer == NULL)
                return NULL;
        }
    }
    return latedSymPointer;
}

Symbol* checkStatement(TreeNode* root, Symbol* latedSymPointer, FILE* fp, Bool* isHasReturn)
{
    root = root->ChildPtrList[0];
    switch (root->nodetype)
    {
    case NT_BLOCK:
    {
        // 创建子符号表
        latedSymPointer = buildNewSymbol(latedSymPointer, False);
        latedSymPointer->symType = ST_TABLE;
        Symbol* childSym_p = buildNewSymbol(latedSymPointer, True);
        childSym_p = checkBlock(root, childSym_p, fp, isHasReturn);
        if (childSym_p == NULL)
            return NULL;
        break;
    }
    case NT_LOCAL_VAR_DECL:
    {
        ValueType valType = getValueType(root->ChildPtrList[0]->ChildPtrList[0]->tp->type);
        for (int i = 1; i < root->ChildCurrentIndex; i += 2)
        {
            TreeNode* tempNode = root->ChildPtrList[i];
            token* tp = tempNode->ChildPtrList[0]->tp;
            updateScanIndex(tp);
            // 查看id是否存在
            if (isSymbolExist(latedSymPointer, tp, ST_VAR))
            {
                throwError(RowScanIndex, ColScanIndex, "A variable with the same name already exists in this scope.");
                return NULL;
            }
            latedSymPointer = buildNewSymbol(latedSymPointer, False);
            latedSymPointer->symType = ST_VAR;
            latedSymPointer->tp = tp;
            latedSymPointer->valType = valType;
            latedSymPointer->isVariable = True;
            buildNewStmtValue(&(latedSymPointer->newName));
            // 存在赋值语句，运行
            if (tempNode->ChildCurrentIndex > 1)
            {
                latedSymPointer = checkBoolMultiExpr(tempNode->ChildPtrList[2],
                                                     latedSymPointer, &(latedSymPointer->newName),
                    &(latedSymPointer->valType), fp);
                if (latedSymPointer == NULL)
                    return NULL;
            }
        }
        updateScanIndex(root->ChildPtrList[root->ChildCurrentIndex - 1]->tp);
        break;
    }
    case NT_ASSIGN_STMT:
    {
        char* tempName = NULL;
        ValueType valType;
        latedSymPointer = checkAssignExpr(root->ChildPtrList[0],
                                          latedSymPointer, &tempName, &valType, fp);
        if (latedSymPointer == NULL)
            return NULL;
        updateScanIndex(root->ChildPtrList[root->ChildCurrentIndex - 1]->tp);
        break;
    }
    case NT_RETURN_STMT:
    {
        // 更新标记
        *isHasReturn = True;
        updateScanIndex(root->ChildPtrList[0]->tp);
        // 获取函数返回值的类型
        Symbol* sp = latedSymPointer;
        while (sp->symType != ST_HEAD)
            sp = sp->prevSymPointer;
        ValueType funcValType = sp->parentSymPointer->valType;
        // 获取返回值
        char* tempName = NULL;
        latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[1],
                                             latedSymPointer, &tempName, &funcValType, fp);
        if (latedSymPointer == NULL)
            return NULL;
        fprintf(fp, "    return %s\n", tempName);
        updateScanIndex(root->ChildPtrList[root->ChildCurrentIndex - 1]->tp);
        break;
    }
    case NT_IF_STMT:
    {
        //     ... (t = BoolMultiExpr)
        //     ifFalse t goto L1
        //     Statement1
        //     goto L2
        // L1: (Statement2)
        // L2: ...
        updateScanIndex(root->ChildPtrList[1]->tp);
        char* label1 = NULL;
        buildNewLabel(&label1);
        // 执行BoolMultiExpr
        char* tempName = NULL;
        ValueType tempValType = VT_BOOL;
        latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[2],
                                             latedSymPointer, &tempName, &tempValType, fp);
        if (latedSymPointer == NULL)
            return NULL;
        // 输出判断
        fprintf(fp, "    ifFalse %s goto %s\n", tempName, label1);
        updateScanIndex(root->ChildPtrList[3]->tp);
        // 执行statement
        latedSymPointer = checkStatement(root->ChildPtrList[4], latedSymPointer, fp, isHasReturn);
        if (latedSymPointer == NULL)
            return NULL;
        // 若为if-else形式，继续执行判断
        if (root->ChildCurrentIndex > 5)
        {
            char* label2 = NULL;
            buildNewLabel(&label2);
            fprintf(fp, "    goto %s\n", label2);
            fprintf(fp, "Label %s\n", label1);
            updateScanIndex(root->ChildPtrList[5]->tp);
            latedSymPointer = checkStatement(root->ChildPtrList[6], latedSymPointer, fp, isHasReturn);
            if (latedSymPointer == NULL)
                return NULL;
            fprintf(fp, "Label %s\n", label2);
        }
        else
            fprintf(fp, "Label %s\n", label1);
        break;
    }
    case NT_WHILE_STMT:
    {
        // L1: ...(t = BoolMultiExpr)
        //     ifFalse t goto L2
        //     ...(Statement)
        //     goto L1
        // L2: ...
        updateScanIndex(root->ChildPtrList[1]->tp);
        char* label1 = NULL;
        buildNewLabel(&label1);
        char* label2 = NULL;
        buildNewLabel(&label2);
        fprintf(fp, "Label %s\n", label1);
        char* tempName = NULL;
        ValueType tempValType = VT_BOOL;
        latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[2], latedSymPointer,
                                             &tempName, &tempValType, fp);
        if (latedSymPointer == NULL)
            return NULL;
        fprintf(fp, "    ifFalse %s goto %s\n", tempName, label2);
        updateScanIndex(root->ChildPtrList[3]->tp);
        latedSymPointer = checkStatement(root->ChildPtrList[4], latedSymPointer, fp, isHasReturn);
        if (latedSymPointer == NULL)
            return NULL;
        fprintf(fp, "    goto %s\n", label1);
        fprintf(fp, "Label %s\n", label2);
        break;
    }
    case NT_WRITE_STMT:
    {
        updateScanIndex(root->ChildPtrList[1]->tp);
        char* tempName = NULL;
        ValueType valType = VT_ANY;
        latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[2], latedSymPointer,
                                             &tempName, &valType, fp);
        if (latedSymPointer == NULL)
            return NULL;
        fprintf(fp, "    write %s, %s\n", tempName, root->ChildPtrList[4]->tp->str);
        updateScanIndex(root->ChildPtrList[root->ChildCurrentIndex - 1]->tp);
        break;
    }
    case NT_READ_STMT:
    {
        updateScanIndex(root->ChildPtrList[1]->tp);
        token* tp = root->ChildPtrList[2]->tp;
        updateScanIndex(tp);
        // 查找id
        Symbol* sp = findSymbol(latedSymPointer, tp->str, ST_VAR);
        if (sp == NULL)
        {
            throwError(RowScanIndex, ColScanIndex, "The variable is not defined.");
            return NULL;
        }
        fprintf(fp, "    read %s, %s\n", sp->newName, root->ChildPtrList[4]->tp->str);
        updateScanIndex(root->ChildPtrList[root->ChildCurrentIndex - 1]->tp);
        break;
    }
    default:
    {
        throwError(RowScanIndex, ColScanIndex, "Unknown Error.@3");
        return NULL;
        break;
    }
    }
    return latedSymPointer;
}

Symbol* checkBoolMultiExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    // [1]: 只有1个BoolMidExpr时直接执行
    if (root->ChildCurrentIndex==1)
    {
        latedSymPointer = checkBoolMidExpr(root->ChildPtrList[0], latedSymPointer, requiredName, requiredValType, fp);
        if (latedSymPointer==NULL)
            return NULL;
    }
    // [2]: 多个时，要生成bool表达式。设 x AND y AND ... AND z
    //       ...(get x)
    //     ifFalse  x  goto L1
    //       ...(get y)
    //     ifFalse  y  goto L1
    //       ...
    //     ifFalse ... goto L1
    //       ...(get z)
    //     ifFalse  z  goto L1
    //     t = true
    //     goto L2
    // L1: t = false
    // L2: ...(此后t即为该表达式值)
    else
    {
        if (*requiredValType!=VT_BOOL&&*requiredValType!=VT_ANY)
        {
            throwError(RowScanIndex, ColScanIndex, "Type doesn't match");
            return NULL;
        }
        // 设置要求属性为VT_BOOL
        *requiredValType = VT_BOOL;
        char* label1 = NULL;
        buildNewLabel(&label1);
        char* label2 = NULL;
        buildNewLabel(&label2);
        for (int childIndex = 0; childIndex<root->ChildCurrentIndex; childIndex+=2)
        {
            // 更新位置
            if (childIndex!=0)
                updateScanIndex(root->ChildPtrList[childIndex-1]->tp);
            // 通过tempName套取checkBoolMidExpr的输出变量名
            char* tempName = NULL;
            // 解析BoolMidExpr
            latedSymPointer = checkBoolMidExpr(root->ChildPtrList[childIndex], latedSymPointer, &tempName, requiredValType, fp);
            if (latedSymPointer==NULL)
                return NULL;
            fprintf(fp, "    ifFalse %s goto %s\n", tempName, label1);
        }
        // 名字没要求，获取一个临时名字
        if (*requiredName==NULL)
            buildNewTempValue(requiredName);
        fprintf(fp, "    %s = true\n", *requiredName);
        fprintf(fp, "    goto %s\n", label2);
        fprintf(fp, "Label %s\n", label1);
        fprintf(fp, "    %s = false\n", *requiredName);
        fprintf(fp, "Label %s\n", label2);
    }
    return latedSymPointer;
}

Symbol* checkBoolMidExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    // [1]: 只有1个BoolMidExpr时直接执行
    if (root->ChildCurrentIndex==1)
    {
        latedSymPointer = checkBoolExpression(root->ChildPtrList[0], latedSymPointer, requiredName, requiredValType, fp);
        if (latedSymPointer==NULL)
            return NULL;
    }
    // [2]: 多个时，要生成bool表达式。设 x OR y OR ... OR z
    //       ...(get x)
    //     if  x  goto L1
    //       ...(get y)
    //     if  y  goto L1
    //       ...
    //     if ... goto L1
    //       ...(get z)
    //     if  z  goto L1
    //     t = false
    //     goto L2
    // L1: t = true
    // L2: ...(此后t即为该表达式值)
    else
    {
        if (*requiredValType!=VT_BOOL&&*requiredValType!=VT_ANY)
        {
            throwError(RowScanIndex, ColScanIndex, "Type doesn't match");
            return NULL;
        }
        *requiredValType = VT_BOOL;
        char* label1 = NULL;
        buildNewLabel(&label1);
        char* label2 = NULL;
        buildNewLabel(&label2);
        for (int childIndex = 0; childIndex<root->ChildCurrentIndex; childIndex+=2)
        {
            // 更新位置
            if (childIndex!=0)
                updateScanIndex(root->ChildPtrList[childIndex-1]->tp);
            // 通过tempName套取checkBoolMidExpr的输出变量名
            char* tempName = NULL;
            // 解析BoolMidExpr
            latedSymPointer = checkBoolMidExpr(root->ChildPtrList[childIndex], latedSymPointer, &tempName, requiredValType, fp);
            if (latedSymPointer==NULL)
                return NULL;
            fprintf(fp, "    if %s goto %s\n", tempName, label1);
        }
        // 名字没要求，获取一个临时名字
        if (*requiredName==NULL)
            buildNewTempValue(requiredName);
        fprintf(fp, "    %s = false\n", *requiredName);
        fprintf(fp, "    goto %s\n", label2);
        fprintf(fp, "Label %s\n", label1);
        fprintf(fp, "    %s = true\n", *requiredName);
        fprintf(fp, "Label %s\n", label2);
    }
    return latedSymPointer;
}

Symbol* checkBoolExpression(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    if (root->ChildCurrentIndex==1)
        latedSymPointer = checkExpression(root->ChildPtrList[0], latedSymPointer, requiredName, requiredValType, fp);
    else
    {
        if (*requiredValType!=VT_ANY&&*requiredValType!=VT_BOOL)
        {
            throwError(RowScanIndex, ColScanIndex, "The types not matched.");
            return NULL;
        }
        *requiredValType = VT_BOOL;
        char* name1 = NULL;
        char* name2 = NULL;
        ValueType valType1 = VT_ANY;
        ValueType valType2 = VT_ANY;
        latedSymPointer = checkExpression(root->ChildPtrList[0], latedSymPointer, &name1, &valType1, fp);
        if (latedSymPointer==NULL)
            return NULL;
        // 更新位置
        updateScanIndex(root->ChildPtrList[1]->tp);
        latedSymPointer = checkExpression(root->ChildPtrList[2], latedSymPointer, &name2, &valType2, fp);
        if (latedSymPointer==NULL)
            return NULL;

        if (valType1==VT_INT)
        {
            switch (valType2)
            {
            case VT_INT:
            {
                if (outputCompareCode(requiredName, name1, name2, root->ChildPtrList[1]->tp, fp) == False)
                    return NULL;
                break;
            }
            case VT_REAL:
            {
                char* newName1 = NULL;
                if (outputTypeTransformCode(&newName1, name1, valType1, VT_REAL, fp) == False)
                    return NULL;
                if (outputCompareCode(requiredName, newName1, name2, root->ChildPtrList[1]->tp, fp) == False)
                    return NULL;
                break;
            }
            default:
            {
                throwParserError(root->ChildPtrList[1]->tp, "The types can't be compared.");
                return NULL;
                break;
            }
            }
        }
        else if (valType1==VT_REAL)
        {
            switch (valType2)
            {
            case VT_INT:
            {
                char* newName2 = NULL;
                if (outputTypeTransformCode(&newName2, name2, valType2, VT_REAL, fp) == False)
                    return NULL;
                if (outputCompareCode(requiredName, name1, newName2, root->ChildPtrList[1]->tp, fp) == False)
                    return NULL;
                break;
            }
            case VT_REAL:
            {
                if (outputCompareCode(requiredName, name1, name2, root->ChildPtrList[1]->tp, fp) == False)
                    return NULL;
                break;
            }
            default:
            {
                throwParserError(root->ChildPtrList[1]->tp, "The types can't be compared.");
                return NULL;
                break;
            }
            }
        }
        else if (valType1==VT_BOOL)
        {
            if (valType2==VT_BOOL)
            {
                if (outputCompareCode(requiredName, name1, name2, root->ChildPtrList[1]->tp, fp)==False)
                    return NULL;
            }
            else
            {
                throwParserError(root->ChildPtrList[1]->tp, "The types can't be compared.");
                return NULL;
            }
        }
        else
        {
            throwParserError(root->ChildPtrList[1]->tp, "The types can't be compared.");
            return NULL;
        }
    }
    return latedSymPointer;
}

Symbol* checkExpression(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    if (root->ChildCurrentIndex==1)
        latedSymPointer = checkMultiplicativeExpr(root->ChildPtrList[0], latedSymPointer, requiredName, requiredValType, fp);
    else 
    {
        char* name1 = NULL;
        ValueType valType1 = VT_ANY;
        for (int childIndex = 0; childIndex < root->ChildCurrentIndex; childIndex+=2)
        {
            char* name2 = NULL;
            ValueType valType2 = VT_ANY;
            latedSymPointer = checkMultiplicativeExpr(root->ChildPtrList[childIndex], latedSymPointer, &name2, &valType2, fp);
            if (latedSymPointer==NULL)
                return NULL;
            if (childIndex==0)
            {
                name1 = name2;
                valType1 = valType2;
            }
            else 
            {
                updateScanIndex(root->ChildPtrList[childIndex-1]->tp);
                // 未到最后，都用临时变量名
                if (childIndex < root->ChildCurrentIndex-1)
                {
                    char* newName = NULL;
                    ValueType newValType = VT_ANY;
                    if (outputBinaryOperaterCode(&newName, &newValType, name1, valType1, name2, valType2, root->ChildPtrList[childIndex-1]->tp, fp)==False)
                        return NULL;
                    name1 = newName;
                    valType1 = newValType;
                }
                // 到最后，用requiredName的变量名
                else
                {
                    if (outputBinaryOperaterCode(requiredName, requiredValType, name1, valType1, name2, valType2, root->ChildPtrList[childIndex-1]->tp, fp)==False)
                        return NULL;
                }
            }
        }
    }
    return latedSymPointer;
}

Symbol* checkMultiplicativeExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    if (root->ChildCurrentIndex==1)
        latedSymPointer = checkPrimaryExpr(root->ChildPtrList[0], latedSymPointer, requiredName, requiredValType, fp);
    else 
    {
        char* name1 = NULL;
        ValueType valType1 = VT_ANY;
        for (int childIndex = 0; childIndex < root->ChildCurrentIndex; childIndex+=2)
        {
            char* name2 = NULL;
            ValueType valType2 = VT_ANY;
            latedSymPointer = checkPrimaryExpr(root->ChildPtrList[childIndex], latedSymPointer, &name2, &valType2, fp);
            if (latedSymPointer==NULL)
                return NULL;
            if (childIndex==0)
            {
                name1 = name2;
                valType1 = valType2;
            }
            else 
            {
                updateScanIndex(root->ChildPtrList[childIndex-1]->tp);
                // 未到最后，都用临时变量名
                if (childIndex < root->ChildCurrentIndex-1)
                {
                    char* newName = NULL;
                    ValueType newValType = VT_ANY;
                    if (outputBinaryOperaterCode(&newName, &newValType, name1, valType1, name2, valType2, root->ChildPtrList[childIndex-1]->tp, fp)==False)
                        return NULL;
                    name1 = newName;
                    valType1 = newValType;
                }
                // 到最后，用requiredName的变量名
                else
                {
                    if (outputBinaryOperaterCode(requiredName, requiredValType, name1, valType1, name2, valType2, root->ChildPtrList[childIndex-1]->tp, fp)==False)
                        return NULL;
                }
            }
        }
    }
    return latedSymPointer;
}

Symbol* checkPrimaryExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    // 更新位置
    updateScanIndex(root->ChildPtrList[0]->tp);
    int childIndex = 0;
    // 若起始为!，则对输出类型有要求
    if (root->ChildPtrList[childIndex]->tp->type==NOT)
    {
        if (*requiredValType!=VT_BOOL&&*requiredValType!=VT_ANY)
        {
            throwError(RowScanIndex, ColScanIndex, "The type doesn't match.");
            return NULL;
        }
        *requiredValType = VT_BOOL;
        childIndex++;
    }
    // 若起始为-，则对输出类型有要求
    else if (root->ChildPtrList[childIndex]->tp->type==SUB)
    {
        if (*requiredValType!=VT_INT && *requiredValType!=VT_REAL && *requiredValType!=VT_ANY)
        {
            throwError(RowScanIndex, ColScanIndex, "The type doesn't match.");
            return NULL;
        }
        childIndex++;
    }

    char *tempName = NULL;
    ValueType valType = VT_ANY;
    // 更新位置
    updateScanIndex(root->ChildPtrList[childIndex]->tp);
    switch (root->ChildPtrList[childIndex]->tp->type)
    {
    case NUMBER:
    {
        tempName = root->ChildPtrList[childIndex]->tp->str;
        if (isNumInt(tempName) == True)
            valType = VT_INT;
        else
            valType = VT_REAL;
        // 转换输出
        if (root->ChildPtrList[0]->tp->type == NOT)
            outputNotCode(requiredName, tempName, valType, fp);
        else if (root->ChildPtrList[0]->tp->type == SUB)
        {
            updateScanIndex(root->ChildPtrList[0]->tp);
            if (*requiredValType == VT_ANY)
                *requiredValType = valType;
            if (*requiredValType == valType)
                outputMinusCode(requiredName, tempName, fp);
            else
            {
                char* tempName2 = NULL;
                outputMinusCode(&tempName2, tempName, fp);
                outputTypeTransformCode(requiredName, tempName2, valType, *requiredValType, fp);
            }
        }
        else
        {
            if (*requiredValType == VT_ANY)
                *requiredValType = valType;
            if (outputTypeTransformCode(requiredName, tempName, valType, *requiredValType, fp) == False)
                return NULL;
        }
        break;
    }
    case ID:
    {
        //判断是变量还是函数
        if (root->ChildCurrentIndex - childIndex == 1)
        {// 如果是变量
            char* varName = root->ChildPtrList[childIndex]->tp->str;
            // 查看符号表和祖先符号表是否存在该变量
            const Symbol* p = findSymbol(latedSymPointer, varName, ST_VAR);
            if (p == NULL)
            {
                throwError(RowScanIndex, ColScanIndex, "The variable is not defined.");
                return NULL;
            }
            valType = p->valType;
            tempName = p->newName;// 返回映射后的名字
            // 转换输出
            if (root->ChildPtrList[0]->tp->type == NOT)
                outputNotCode(requiredName, tempName, valType, fp);
            else if (root->ChildPtrList[0]->tp->type == SUB)
            {
                updateScanIndex(root->ChildPtrList[0]->tp);
                // 不是数字类型，报错
                if (valType != VT_INT && valType != VT_REAL)
                {
                    throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
                    return NULL;
                }
                if (*requiredValType == VT_ANY)
                    *requiredValType = valType;
                // 同个类型直接输出，不同类型需要转换
                if (*requiredValType == valType)
                    outputMinusCode(requiredName, tempName, fp);
                else
                {
                    char* tempName2 = NULL;
                    outputMinusCode(&tempName2, tempName, fp);
                    outputTypeTransformCode(requiredName, tempName2, valType, *requiredValType, fp);
                }
            }
            else
            {
                if (*requiredValType == VT_ANY)
                    *requiredValType = valType;
                if (outputTypeTransformCode(requiredName, tempName, valType, *requiredValType, fp) == False)
                    return NULL;
            }
        }
        else
        {// 如果是函数
            char* funcName = root->ChildPtrList[childIndex]->tp->str;
            // 查找是否有该函数
            Symbol* sp = findSymbol(latedSymPointer, funcName, ST_FUNC);
            if (sp == NULL)
            {
                throwError(RowScanIndex, ColScanIndex, "The function is not defined.");
                return NULL;
            }
            tempName = sp->newName;
            valType = sp->valType;
            // 有参数，解析ActualParams
            int count = 0;// 参数个数
            if (root->ChildCurrentIndex - childIndex > 3)
            {
                sp = sp->childSymPointer->nextSymPointer;
                for (int i = 0; i < root->ChildPtrList[childIndex + 2]->ChildCurrentIndex; i+=2)
                {
                    // 不是参数了
                    if (sp == NULL || sp->isParam == False)
                    {
                        throwError(RowScanIndex, ColScanIndex, "The number of parameters is out of range.");
                        return NULL;
                    }
                    TreeNode* tempNode = root->ChildPtrList[childIndex + 2]->ChildPtrList[i];
                    char* tempName2 = NULL;
                    ValueType valType2 = sp->valType;
                    latedSymPointer = checkBoolMultiExpr(tempNode, latedSymPointer, &tempName2, &valType2, fp);
                    if (latedSymPointer == NULL)
                        return NULL;
                    else
                    {
                        fprintf(fp, "    param %s\n", tempName2);
                        sp = sp->nextSymPointer;
                        count++;
                    }
                }
                if (sp != NULL && sp->isParam == True)
                {
                    throwError(RowScanIndex, ColScanIndex, "The number of parameters is insufficient.");
                    return NULL;
                }
            }

            // 调用函数，获得输出变量
            char* tempName2 = NULL;
            buildNewTempValue(&tempName2);
            fprintf(fp, "    %s = call %s, %d\n", tempName2, tempName, count);
            // 转换输出
            if (root->ChildPtrList[0]->tp->type == NOT)
                outputNotCode(requiredName, tempName2, valType, fp);
            else if (root->ChildPtrList[0]->tp->type == SUB)
            {
                updateScanIndex(root->ChildPtrList[0]->tp);
                // 不是数字类型，报错
                if (valType != VT_INT && valType != VT_REAL)
                {
                    throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
                    return NULL;
                }
                if (*requiredValType == VT_ANY)
                    *requiredValType = valType;
                // 同个类型直接输出，不同类型需要转换
                if (*requiredValType == valType)
                    outputMinusCode(requiredName, tempName2, fp);
                else
                {
                    char* tempName3 = NULL;
                    outputMinusCode(&tempName3, tempName2, fp);
                    outputTypeTransformCode(requiredName, tempName3, valType, *requiredValType, fp);
                }
            }
            else
            {
                if (*requiredValType == VT_ANY)
                    *requiredValType = valType;
                if (outputTypeTransformCode(requiredName, tempName2, valType, *requiredValType, fp) == False)
                    return NULL;
            }
        }
        break;
    }
    case TRUE:
    case FALSE:
    {
        if (root->ChildPtrList[childIndex]->tp->type == TRUE)
            tempName = "True";
        else
            tempName = "False";
        valType = VT_BOOL;
        // 转换输出
        if (root->ChildPtrList[0]->tp->type == NOT)
            outputNotCode(requiredName, tempName, valType, fp);
        else if (root->ChildPtrList[0]->tp->type == SUB)
        {
            updateScanIndex(root->ChildPtrList[0]->tp);
            throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
            return NULL;
        }
        else
        {
            if (*requiredValType != VT_ANY && *requiredValType != VT_BOOL)
            {
                throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
                return NULL;
            }
            *requiredValType = valType;
            if (outputTypeTransformCode(requiredName, tempName, valType, *requiredValType, fp) == False)
                return NULL;
        }
        break;
    }
    case LP:
    {
        if (root->ChildPtrList[0]->tp->type == NOT)
        {
            updateScanIndex(root->ChildPtrList[0]->tp);
            latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[childIndex + 1], latedSymPointer, &tempName, &valType, fp);
            if (latedSymPointer == NULL)
                return NULL;
            outputNotCode(requiredName, tempName, valType, fp);
        }
        else if (root->ChildPtrList[0]->tp->type == SUB)
        {
            latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[childIndex + 1], latedSymPointer, &tempName, &valType, fp);
            if (latedSymPointer == NULL)
                return NULL;
            updateScanIndex(root->ChildPtrList[0]->tp);
            // 不是数字类型，报错
            if (valType != VT_INT && valType != VT_REAL)
            {
                throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
                return NULL;
            }
            if (*requiredValType == VT_ANY)
                *requiredValType = valType;
            // 同个类型直接输出，不同类型需要转换
            if (*requiredValType == valType)
                outputMinusCode(requiredName, tempName, fp);
            else
            {
                char* tempName2 = NULL;
                outputMinusCode(&tempName2, tempName, fp);
                outputTypeTransformCode(requiredName, tempName2, valType, *requiredValType, fp);
            }
        }
        else
        {
            latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[childIndex + 1], latedSymPointer, requiredName, requiredValType, fp);
            if (latedSymPointer == NULL)
                return NULL;
        }
        break;
    }
    default:
    {
        throwError(ColScanIndex, RowScanIndex, "Unknown error.@2");
        return NULL;
        break;
    }
    }
    return latedSymPointer;
}

Symbol* checkAssignExpr(TreeNode* root, Symbol* latedSymPointer, char** requiredName, ValueType* requiredValType, FILE* fp)
{
    char* varName = root->ChildPtrList[0]->tp->str;
    // 查看符号表和祖先符号表是否存在该变量
    const Symbol* p = findSymbol(latedSymPointer, varName, ST_VAR);
    if (p==NULL)
    {
        throwError(RowScanIndex, ColScanIndex, "The variable is not defined.");
        return NULL;
    }
    ValueType valType = p->valType;
    char* tempName = p->newName;// 返回映射后的名字
    if (root->ChildCurrentIndex>1)
    {
        latedSymPointer = checkBoolMultiExpr(root->ChildPtrList[2], latedSymPointer, &tempName, &valType, fp);
        if (latedSymPointer==NULL)
            return NULL;
    }
    return latedSymPointer;
}

Bool outputCompareCode(char** requiredName, char* name1, char* name2, token* tp, FILE* fp)
{
    char* label1 = NULL;
    buildNewLabel(&label1);
    char* label2 = NULL;
    buildNewLabel(&label2);
    if (*requiredName==NULL)
    {
        *requiredName = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(*requiredName, "#TV%d", SymbolNum[2]++);
    }
    fprintf(fp, "    if %s ", name1);
    switch (tp->type)
    {
    case EQUAL:
        fprintf(fp, "==");
        break;
    case UNEQUAL:
        fprintf(fp, "!=");
        break;
    case GEQ:
        fprintf(fp, ">=");
        break;
    case LEQ:
        fprintf(fp, "<=");
        break;
    case GT:
        fprintf(fp, ">");
        break;
    case LT:
        fprintf(fp, "<");
        break;
    default :
        throwParserError(tp, "Unexpected operator.");
        return False;
        break;
    }
    fprintf(fp, " %s goto %s\n", name2, label1);
    fprintf(fp, "    %s = false\n", *requiredName);
    fprintf(fp, "    goto %s\n", label2);
    fprintf(fp, "Label %s\n", label1);
    fprintf(fp, "    %s = true\n", *requiredName);
    fprintf(fp, "Label %s\n", label2);
    return True;
}
/*
Bool outputToBoolCode(char** requiredName, char* name, ValueType valType, FILE* fp)
{
    if (*requiredName==NULL)
    {
        *requiredName = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(*requiredName, "#TV%d", SymbolNum[2]++);
    }
    switch (valType)
    {
    case VT_INT:
    case VT_REAL:
        char* label1 = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(label1, "#L%d", SymbolNum[5]++);
        char* label2 = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(label2, "#L%d", SymbolNum[5]++);
        // 逻辑：
        //     if x == 0 goto L1
        //     t = true
        //     goto L2
        // L1: t = false
        // L2: ...(此后t即为x)
        fprintf(fp, "if %s == 0 goto %s\n", name, label1);
        fprintf(fp, "%s = true\n", *requiredName);
        fprintf(fp, "goto %s\n", label2);
        fprintf(fp, "Label %s", label1);
        fprintf(fp, "%s = false\n", *requiredName);
        fprintf(fp, "Label %s", label2);
        break;
    case VT_BOOL:
        fprintf(fp, "%s = %s\n", *requiredName, name);
        break;
    default:
        throwError(RowScanIndex, ColScanIndex, "The type can't be changed to Bool.");
        return False;
        break;
    }
    return True;
}

Bool outputToIntCode(char** requiredName, char* name, ValueType valType, FILE* fp)
{

}

Bool outputToRealCode(char** requiredName, char* name, ValueType valType, FILE* fp)
{
    
}
*/

Bool outputTypeTransformCode(char** requiredName, char* name, ValueType valType, ValueType requiredType, FILE* fp)
{
    switch (requiredType)
    {
    case VT_INT:
        switch (valType)
        {
        case VT_REAL:
            buildNewTempValue(requiredName);
            fprintf(fp, "    %s = (int)%s\n", *requiredName, name);
            break;
        case VT_INT:
            if (*requiredName == NULL)
                *requiredName = name;
            else
                fprintf(fp, "    %s = %s\n", *requiredName, name);
            break;
        default:
            throwError(RowScanIndex, ColScanIndex, "The type can't be changed to INT.");
            return False;
            break;
        }
        return True;
        break;
    case VT_REAL:
        switch (valType)
        {
        case VT_INT:
            buildNewTempValue(requiredName);
            fprintf(fp, "    %s = (real)%s\n", *requiredName, name);
            break;
        case VT_REAL:
            if (*requiredName == NULL)
                *requiredName = name;
            else
                fprintf(fp, "    %s = %s\n", *requiredName, name);
            break;
        default:
            throwError(RowScanIndex, ColScanIndex, "The type can't be changed to REAL.");
            return False;
            break;
        }
        return True;
        break;
    case VT_BOOL:
        switch (valType)
        {
        case VT_INT:
        case VT_REAL:
        {
            buildNewTempValue(requiredName);
            char* label1 = (char*)calloc(MAXBUFLEN, sizeof(char));
            sprintf(label1, "#L%d", SymbolNum[5]++);
            char* label2 = (char*)calloc(MAXBUFLEN, sizeof(char));
            sprintf(label2, "#L%d", SymbolNum[5]++);
            // 逻辑：
            //     if x == 0 goto L1
            //     t = true
            //     goto L2
            // L1: t = false
            // L2: ...(此后t即为x)
            fprintf(fp, "    if %s == 0 goto %s\n", name, label1);
            fprintf(fp, "    %s = true\n", *requiredName);
            fprintf(fp, "    goto %s\n", label2);
            fprintf(fp, "Label %s", label1);
            fprintf(fp, "    %s = false\n", *requiredName);
            fprintf(fp, "Label %s", label2);
            break;
        }
        case VT_BOOL:
        {
            if (*requiredName == NULL)
                *requiredName = name;
            else
                fprintf(fp, "    %s = %s\n", *requiredName, name);
            break;
        }
        default:
        {
            throwError(RowScanIndex, ColScanIndex, "The type can't be changed to BOOL.");
            return False;
            break;
        }
        }
        return True;
        break;
    default:
        throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
        return False;
        break;
    }
    return True;
}

Bool outputBinaryOperaterCode(char** requiredName, ValueType* newType, char* name1, ValueType valType1, char* name2, ValueType valType2, token* tp, FILE* fp)
{
    buildNewTempValue(requiredName);

    char *cp;
    switch (tp->type)
    {
    case ADD:
        cp = "+";
        break;
    case SUB:
        cp = "-";
        break;
    case MUL:
        cp = "*";
        break;
    case DIV:
        cp = "/";
        break;
    case MOD:
        cp = "\%";
        break;
    default:
        throwParserError(tp, "Unexpected operator.");
        return False;
        break;
    }

    switch (valType1)
    {
    case VT_INT:
        switch (valType2)
        {
        case VT_INT:
            switch (*newType)
            {
            case VT_INT:
            case VT_ANY:
            {
                fprintf(fp, "    %s = %s %s %s\n", *requiredName, name1, cp, name2);
                *newType = VT_INT;
                break;
            }
            case VT_REAL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                fprintf(fp, "    %s = %s %s %s\n", tempName, name1, cp, name2);
                fprintf(fp, "    %s = (real)%s\n", *requiredName, tempName);
                break;
            }
            case VT_BOOL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                fprintf(fp, "    %s = %s %s %s\n", tempName, name1, cp, name2);
                outputTypeTransformCode(requiredName, tempName, VT_INT, VT_BOOL, fp);
                break;
            }
            default:
            {
                throwParserError(tp, "The types can't be calculated.");
                return False;
                break;
            }
            }
            break;
        case VT_REAL:
            switch (*newType)
            {
            case VT_INT:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                char* tempName2 = NULL;
                buildNewTempValue(&tempName2);
                fprintf(fp, "    %s = (real)%s\n", tempName, name1);
                fprintf(fp, "    %s = %s %s %s\n", tempName2, tempName, cp, name2);
                fprintf(fp, "    %s = (int)%s\n", *requiredName, tempName2);
                break;
            }
            case VT_ANY:
            case VT_REAL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                fprintf(fp, "    %s = (real)%s\n", tempName, name1);
                fprintf(fp, "    %s = %s %s %s\n", *requiredName, tempName, cp, name2);
                *newType = VT_REAL;
                break;
            }
            case VT_BOOL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                char* tempName2 = NULL;
                buildNewTempValue(&tempName2);
                fprintf(fp, "    %s = (real)%s\n", tempName, name1);
                fprintf(fp, "    %s = %s %s %s\n", tempName2, tempName, cp, name2);
                outputTypeTransformCode(requiredName, tempName2, VT_REAL, VT_BOOL, fp);
                break;
            }
            default:
            {
                throwParserError(tp, "The types can't be calculated.");
                return False;
                break;
            }
            }
            break;
        default:
        {
            throwParserError(tp, "The types can't be calculated.");
            return False;
            break;
        }
        }
        break;
    case VT_REAL:
        switch (valType2)
        {
        case VT_INT:
            switch (*newType)
            {
            case VT_INT:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                char* tempName2 = NULL;
                buildNewTempValue(&tempName2);
                fprintf(fp, "    %s = (real)%s\n", tempName, name2);
                fprintf(fp, "    %s = %s %s %s\n", tempName2, name1, cp, tempName);
                fprintf(fp, "    %s = (int)%s\n", *requiredName, tempName2);
                break;
            }
            case VT_ANY:
            case VT_REAL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                fprintf(fp, "    %s = (real)%s\n", tempName, name2);
                fprintf(fp, "    %s = %s %s %s\n", *requiredName, name1, cp, tempName);
                *newType = VT_REAL;
                break;
            }
            case VT_BOOL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                char* tempName2 = NULL;
                buildNewTempValue(&tempName2);
                fprintf(fp, "    %s = (real)%s\n", tempName, name2);
                fprintf(fp, "    %s = %s %s %s\n", tempName2, name1, cp, tempName);
                outputTypeTransformCode(requiredName, tempName2, VT_REAL, VT_BOOL, fp);
                break;
            }
            default:
            {
                throwParserError(tp, "The types can't be calculated.");
                return False;
                break;
            }
            }
            break;
        case VT_REAL:
            switch (*newType)
            {
            case VT_REAL:
            case VT_ANY:
            {
                fprintf(fp, "    %s = %s %s %s\n", *requiredName, name1, cp, name2);
                *newType = VT_REAL;
                break;
            }
            case VT_INT:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                fprintf(fp, "    %s = %s %s %s\n", tempName, name1, cp, name2);
                fprintf(fp, "    %s = (int)%s\n", *requiredName, tempName);
                break;
            }
            case VT_BOOL:
            {
                char* tempName = NULL;
                buildNewTempValue(&tempName);
                fprintf(fp, "    %s = %s %s %s\n", tempName, name1, cp, name2);
                outputTypeTransformCode(requiredName, tempName, VT_REAL, VT_BOOL, fp);
                break;
            }
            default:
            {
                throwParserError(tp, "The types can't be calculated.");
                return False;
                break;
            }
            }
            break;
        default:
            throwParserError(tp, "The types can't be calculated.");
            return False;
            break;
        }
        break;
    default:
        throwParserError(tp, "The types can't be calculated.");
        return False;
        break;
    }
    return True;
}

Bool outputNotCode(char** requiredName, char* name, ValueType type, FILE* fp)
{
    buildNewTempValue(requiredName);
    char* label1 = NULL;
    buildNewLabel(&label1);
    char* label2 = NULL;
    buildNewLabel(&label2);
    if (type==VT_BOOL)
        fprintf(fp, "    if %s goto %s\n", name, label1);
    else if (type==VT_INT||type==VT_REAL)
        fprintf(fp, "    ifFalse %s==0 goto %s\n", name, label1);
    else
    {
        throwError(RowScanIndex, ColScanIndex, "The type can't be changed.");
        return False;
    }
    fprintf(fp, "    %s = true\n", *requiredName);
    fprintf(fp, "    goto %s\n", label2);
    fprintf(fp, "Label %s\n", label1);
    fprintf(fp, "    %s = false\n", *requiredName);
    fprintf(fp, "Label %s\n", label2);
    return True;
}

Bool outputMinusCode(char** requiredName, char* name, FILE* fp)
{
    buildNewTempValue(requiredName);
    fprintf(fp, "    %s = minus %s\n", *requiredName, name);
    return True;
}

Bool buildNewStmtValue(char** requiredName)
{
    if (*requiredName==NULL)
    {
        *requiredName = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(*requiredName, "#V%d", SymbolNum[0]++);
    }
    return True;
}

Bool buildNewTempValue(char** requiredName)
{
    if (*requiredName==NULL)
    {
        *requiredName = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(*requiredName, "#TV%d", SymbolNum[2]++);
    }
    return True;
}

Bool buildNewFuncName(char** requiredName)
{
    if (*requiredName==NULL)
    {
        *requiredName = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(*requiredName, "#F%d", SymbolNum[4]++);
    }
    return True;
}

Bool buildNewLabel(char** requiredName)
{
    if (*requiredName==NULL)
    {
        *requiredName = (char*)calloc(MAXBUFLEN, sizeof(char));
        sprintf(*requiredName, "#L%d", SymbolNum[5]++);
    }
    return True;
}

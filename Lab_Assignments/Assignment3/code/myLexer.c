// coding: utf-8
// author: 18308013 ChenJiahao
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "myLexer.h"

token KeyTokens[] = {
    {IF, "IF"},
    {ELSE, "ELSE"},
    {WRITE, "WRITE"},
    {READ, "READ"},
    {RETURN, "RETURN"},
    {BEGIN, "BEGIN"},
    {END, "END"},
    {MAIN, "MAIN"},
    {INT, "INT"},
    {REAL, "REAL"},
    {AND, "AND"},
    {OR, "OR"},
    {WHILE, "WHILE"},
    {BOOL, "BOOL"},
    {TRUE, "TRUE"},
    {FALSE, "FALSE"} };
int KeyTokensLen = sizeof(KeyTokens) / sizeof(KeyTokens[0]);

const char* TokenNames[] = {
    "IF",
    "ELSE",
    "WRITE",
    "READ",
    "RETURN",
    "BEGIN",
    "END",
    "MAIN",
    "INT",
    "REAL",
    "AND",
    "OR",
    "WHILE",
    "BOOL",
    "TRUE",
    "FALSE",
    "SEMICOLON",
    "COMMA",
    "LP",
    "RP",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "ASSIGN",
    "LEQ",
    "GEQ",
    "LT",
    "GT",
    "EQUAL",
    "UNEQUAL",
    "NO",
    "NUMBER",
    "ID",
    "STRING",
    "ERROR",
    "FINISH" };
int TokenNamesLen = sizeof(TokenNames) / sizeof(TokenNames[0]);

// 获取TOKEN类型名
const char* getTokenName(TokenType type)
{
    return TokenNames[type];
}

// 判断是否为空白符号
Bool isBlankChar(char ch)
{
    if (ch == 0x20 || ch == 0x09 || ch == 0x0B || ch == 0x0C || ch == 0x0D || ch == 0x0A)
        return True;
    else
        return False;
}

// 判断是否为数字
Bool isDigit(char ch)
{
    if (ch >= '0' && ch <= '9')
        return True;
    else
        return False;
}

// 判断是否为字母
Bool isAlpha(char ch)
{
    if (ch >= 'a' && ch <= 'z')
        return True;
    else if (ch >= 'A' && ch <= 'Z')
        return True;
    else
        return False;
}

// 获取下一个TOKEN
struct token* getNextToken(FILE* source, int* RowIndexPtr, int* ColIndexPtr)
{
    // 创建并初始化TOKEN
    struct token* tp = (struct token*)calloc(1, sizeof(struct token));
    tp->str = (char*)calloc(MAXBUFLEN, sizeof(char));
    int StrIndex = 0;
    int StrLen = MAXBUFLEN;

    char ch;// 当前字符
    // 初始化DFA
    enum DFAStatus CurrentStatus = DFA_START;
    Bool isSave;// 是否存储该字符
    Bool isRecordLocation = False;// 是否在TOKEN中记录位置
    Bool isUpdateLocation = True;// 是否更新RowIndexPtr和ColIndexPtr存储的位置

    while (True)
    {
        // 获取下一个字符
        ch = fgetc(source);
        // 默认存储该字符
        isSave = True;
        // DFA状态跳转
        switch (CurrentStatus)
        {
        case DFA_START:
            if (isBlankChar(ch) != True)
                isRecordLocation = True;
            if (isBlankChar(ch) == True)
                isSave = False;
            else if (isDigit(ch) == True)
                CurrentStatus = DFA_INTNUM;
            else if (isAlpha(ch) == True)
                CurrentStatus = DFA_ID;
            else if (ch == '"')
                CurrentStatus = DFA_STRING;
            else if (ch == '/')
                CurrentStatus = DFA_DIV_OR_COMMENT;
            else if (ch == ':')
                CurrentStatus = DFA_ASSIGN;
            else if (ch == '=')
                CurrentStatus = DFA_EQUAL;
            else if (ch == '!')
                CurrentStatus = DFA_NOT;
            else if (ch == '>')
                CurrentStatus = DFA_GT;
            else if (ch == '<')
                CurrentStatus = DFA_LT;
            else
            {
                CurrentStatus = DFA_DONE;
                if (ch == ',')
                    tp->type = COMMA;
                else if (ch == ';')
                    tp->type = SEMICOLON;
                else if (ch == '+')
                    tp->type = ADD;
                else if (ch == '-')
                    tp->type = SUB;
                else if (ch == '*')
                    tp->type = MUL;
                else if (ch == '%')
                    tp->type = MOD;
                else if (ch == '(')
                    tp->type = LP;
                else if (ch == ')')
                    tp->type = RP;
                else if (ch == EOF)
                {
                    tp->type = FINISH;
                    isSave = False;
                }
                else
                    tp->type = ERROR;
            }
            break;
        case DFA_ID:
            if (isAlpha(ch) != True && isDigit(ch) != True)
            {
                tp->type = ID;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
                isSave = False;
            }
            break;
        case DFA_INTNUM:
            if (ch == '.')
                CurrentStatus = DFA_INT2REAL;
            else if (isDigit(ch) == False)
            {
                if (isAlpha(ch) == True)
                    tp->type = ERROR;
                else
                {
                    tp->type = NUMBER;
                    isSave = False;
                }
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
            }
            break;
        case DFA_INT2REAL:
            if (isDigit(ch) == True)
                CurrentStatus = DFA_REALNUM;
            else
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
            }
            break;
        case DFA_REALNUM:
            if (isDigit(ch) == False)
            {
                tp->type = NUMBER;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
                isSave = False;
            }
            break;
        case DFA_DIV_OR_COMMENT:
            if (ch == '*')
                CurrentStatus = DFA_COMMENT_START;
            else
            {
                tp->type = DIV;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
                isSave = False;
            }
            break;
        case DFA_COMMENT_START:
            if (ch == '*')
                CurrentStatus = DFA_COMMENT;
            else
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
            }
            break;
        case DFA_COMMENT:
            isSave = False;
            if (ch == '*')
                CurrentStatus = DFA_COMMENT_END;
            else if (ch == EOF)
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
            }
            break;
        case DFA_COMMENT_END:
            isSave = False;
            if (ch == '*')
                CurrentStatus = DFA_COMMENT_END2;
            else if (ch == EOF)
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
            }
            else
                CurrentStatus = DFA_COMMENT;
            break;
        case DFA_COMMENT_END2:
            isSave = False;
            if (ch == '/')
            {
                CurrentStatus = DFA_START;
                free(tp->str);
                tp->str = (char*)calloc(MAXBUFLEN, sizeof(char));
                StrIndex = 0;
                StrLen = MAXBUFLEN;
            }
            else if (ch == EOF)
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
            }
            else
                CurrentStatus = DFA_COMMENT;
            break;
        case DFA_STRING:
            if (ch == '"')
            {
                tp->type = STRING;
                CurrentStatus = DFA_DONE;
            }
            else if (ch == EOF)
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
            }
            break;
        case DFA_ASSIGN:
            if (ch == '=')
            {
                tp->type = ASSIGN;
                CurrentStatus = DFA_DONE;
            }
            else
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
            }
            break;
        case DFA_EQUAL:
            if (ch == '=')
            {
                tp->type = EQUAL;
                CurrentStatus = DFA_DONE;
            }
            else
            {
                tp->type = ERROR;
                CurrentStatus = DFA_DONE;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
            }
            break;
        case DFA_NOT:
            CurrentStatus = DFA_DONE;
            if (ch == '=')
                tp->type = UNEQUAL;
            else
            {
                tp->type = NOT;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
                isSave = False;
            }
            break;
        case DFA_GT:
            CurrentStatus = DFA_DONE;
            if (ch == '=')
                tp->type = GEQ;
            else
            {
                tp->type = GT;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
                isSave = False;
            }
            break;
        case DFA_LT:
            CurrentStatus = DFA_DONE;
            if (ch == '=')
                tp->type = LEQ;
            else
            {
                tp->type = LT;
                if (ch != EOF)
                {
                    fseek(source, -(long)sizeof(char), SEEK_CUR);
                    isUpdateLocation = False;
                }
                isSave = False;
            }
            break;
        case DFA_DONE:
            break;
        default:
            CurrentStatus = DFA_DONE;
            tp->type = ERROR;
            break;
        }
        // 存储TOKEN起始位置
        if (isRecordLocation)
        {
            tp->ColIndex = *ColIndexPtr;
            tp->RowIndex = *RowIndexPtr;
            isRecordLocation = False;
        }
        // 更新计数器记录的位置
        if (isUpdateLocation)
        {
            if (ch == '\n')
            {
                (*RowIndexPtr) += 1;
                (*ColIndexPtr) = 1;
            }
            else
                (*ColIndexPtr) += 1;
        }
        // 存储字符
        if (isSave)
        {
            // 空间不足，开辟新空间
            if (StrIndex >= StrLen)
            {
                StrLen += MAXBUFLEN;
                tp->str = (char*)realloc(tp->str, StrLen * sizeof(char));
            }
            tp->str[StrIndex++] = ch;
        }
        // DFA到达结束状态，结束循环
        if (CurrentStatus == DFA_DONE)
        {
            // 存'\0'
            if (StrIndex >= StrLen)
            {
                StrLen += 1;
                tp->str = (char*)realloc(tp->str, StrLen * sizeof(char));
            }
            tp->str[StrIndex++] = '\0';
            // 若为错误，报错
            if (tp->type == ERROR)
                printf("\n[ERROR] %d:%d: %s\n", tp->RowIndex, tp->ColIndex, tp->str);
            // 将特定ID转换为关键字
            else if (tp->type == ID)
            {
                for (int i = 0; i < KeyTokensLen; i++)
                {
                    if (strcmp(tp->str, KeyTokens[i].str) == 0)
                    {
                        tp->type = KeyTokens[i].type;
                        break;
                    }
                }
            }
            break;
        }

    }
    return tp;
}

// 显示TOKEN
void printToken(struct token* tp)
{
    printf("(%s, %s)\n", TokenNames[tp->type], tp->str);
}
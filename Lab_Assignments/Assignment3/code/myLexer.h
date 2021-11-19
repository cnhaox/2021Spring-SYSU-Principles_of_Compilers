// coding: utf-8
// author: 18308013 ChenJiahao
#ifndef MYLEXER_H
#define MYLEXER_H
#include <stdio.h>
#include "myUtils.h"

// TOKEN类型
typedef enum TokenType
{
    // 关键字
    IF,
    ELSE,
    WRITE,
    READ,
    RETURN,
    BEGIN,
    END,
    MAIN,
    INT,
    REAL,
    AND,
    OR,
    WHILE,
    BOOL,
    TRUE,
    FALSE,
    SEMICOLON,  // ;
    COMMA,      // ,
    LP,         // (
    RP,         // )
    ADD,        // +
    SUB,        // -
    MUL,        // *
    DIV,        // /
    MOD,        // %
    ASSIGN,     // :=
    LEQ,        // <=
    GEQ,        // >=
    LT,         // <
    GT,         // >
    EQUAL,      // ==
    UNEQUAL,    // !=
    NOT,        // !
    NUMBER,     // 数字
    ID,         // 描述符
    STRING,     // 字符串
    // 特殊type
    ERROR,
    FINISH
} TokenType;

// DFA状态类型
typedef enum DFAStatus
{
    DFA_START,
    DFA_ID,
    DFA_INTNUM,
    DFA_INT2REAL,
    DFA_REALNUM,
    DFA_DIV_OR_COMMENT,
    DFA_COMMENT_START,
    DFA_COMMENT,
    DFA_COMMENT_END,
    DFA_COMMENT_END2,
    DFA_STRING,
    DFA_ASSIGN,
    DFA_EQUAL,
    DFA_NOT,
    DFA_GT,
    DFA_LT,
    DFA_DONE
} DFAStatus;

typedef struct token
{
    enum TokenType type; // token类型
    char* str;           // token内容
    int RowIndex;        // 起始行号
    int ColIndex;        // 起始列号
} token;

const char* getTokenName(TokenType type);// 获取TOKEN类型名
Bool isBlankChar(char ch);// 判断是否为空白符号
Bool isDigit(char ch);// 判断是否为数字
Bool isAlpha(char ch);// 判断是否为字母
token* getNextToken(FILE* source, int* RowIndexPtr, int* ColIndexPtr);// 获取下一个TOKEN
void printToken(token* tp);// 显示TOKEN

# endif
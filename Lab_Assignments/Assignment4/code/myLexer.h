// coding: utf-8
// author: 18308013 ChenJiahao
#ifndef MYLEXER_H
#define MYLEXER_H
#include <stdio.h>
#include "myStructure.h"
#include "myUtils.h"

const char* getTokenName(TokenType type);// 获取TOKEN类型名
Bool isBlankChar(char ch);// 判断是否为空白符号
Bool isDigit(char ch);// 判断是否为数字
Bool isAlpha(char ch);// 判断是否为字母
token* getNextToken(FILE* source, int* RowIndexPtr, int* ColIndexPtr);// 获取下一个TOKEN
void printToken(token* tp);// 显示TOKEN
void fprintToken(struct token* tp, FILE* fp);
# endif
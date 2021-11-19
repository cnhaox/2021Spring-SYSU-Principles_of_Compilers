// coding: utf-8
// author: 18308013 ChenJiahao
#ifndef MYUTILS_H
#define MYUTILS_H
#include "myStructure.h"
#include "myLexer.h"

#define True 1
#define False 0
#define MAXBUFLEN 32
#define MAXMESLEN 200

void Exit();// 退出函数
void throwParserError(token* tp, const char* cp);// 显示错误
void throwError(int row, int col, const char* cp);// 显示错误

#endif
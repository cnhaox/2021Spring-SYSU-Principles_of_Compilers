#include <stdio.h>
#include <stdlib.h>
#include "myUtils.h"
// 退出函数
void Exit()
{
    system("pause");
    exit(0);
}
// 显示错误
void throwParserError(token* tp, const char* cp)
{
    printf("[ERROR] %d:%d: %s\n", tp->RowIndex, tp->ColIndex, cp);
}

void throwError(int row, int col, const char* cp)
{
    printf("[ERROR] %d:%d: %s\n", row, col, cp);
}

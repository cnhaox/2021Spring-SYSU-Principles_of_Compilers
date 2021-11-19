// coding: utf-8
// author: 18308013 ChenJiahao
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myLexer.h"
#include "myParser.h"

char SourceFilePath[150];
int main()
{
    printf("-----------------------------------------\n");
    printf("|   TINY+ v0.4 :: 18308013 ChenJiahao   |\n");
    printf("-----------------------------------------\n");
    printf("[INFO] Welcome to use TINY+ compiler!\n");
    printf("[INFO] Please input your source file full path: \n[INPUT] ");
    scanf("%s", SourceFilePath);
    FILE* source;
    while ((source = fopen(SourceFilePath, "r")) == NULL)
    {
        printf("[ERROR] Open file %s failed!\n", SourceFilePath);
        char choice = 0;
        while (choice != 'y' && choice != 'n')
        {
            printf("[INFO] Do you want to input again?(y/n): \n[INPUT] ");
            scanf("%c", &choice);
        }
        if (choice == 'y')
        {
            printf("[INFO] Please input your source file full path: \n[INPUT] ");
            scanf("%s", SourceFilePath);
        }
        else
        {
            printf("[INFO] Bye!\n");
            system("pause");
            return 0;
        }
    }
    printf("[INFO] Lexical analysis: \n\n");
    int RowIndex = 1, ColIndex = 1;
    token* tp;
    token** tpArray = (struct token**)calloc(MAXBUFLEN, sizeof(struct token*));
    int TokenIndex = 0;
    int MaxTokenNum = MAXBUFLEN;
    Bool isFinish = False;
    while (True)
    {
        tp = getNextToken(source, &RowIndex, &ColIndex);
        if (TokenIndex >= MaxTokenNum)
        {
            MaxTokenNum += MAXBUFLEN;
            tpArray = (struct token**)realloc(tpArray, MaxTokenNum * sizeof(struct token*));
        }
        tpArray[TokenIndex++] = tp;
        if (tp->type == ERROR || tp->type == FINISH)
        {
            if (tp->type == FINISH)
                isFinish = True;
            else
                printf("\n[INFO] Lexical analysis interrupted.\n");
            break;
        }
        else
            printToken(tp);
    }
    MaxTokenNum = TokenIndex;
    tpArray = (struct token**)realloc(tpArray, MaxTokenNum * sizeof(struct token*));
    if (isFinish)
    {
        printf("\n[INFO] Lexical analysis Finished.\n");
        printf("[INFO] Parser analysis: \n\n");
        TreeNode* root = analyseProgram(tpArray, MaxTokenNum);
        if (root == NULL)
        {
            printf("\n[INFO] Parser analysis interrupted.\n");
            system("pause");
            return 0;
        }
        int MaxDepth = getParserTreeMaxDepth(root);
        Bool* PrintFlagPtr = (Bool*)calloc(MaxDepth, sizeof(Bool));
        printParserTree(root, 1, PrintFlagPtr);
        free(PrintFlagPtr);
        printf("\n[INFO] Parser analysis Finished.\n");
    }
    system("pause");
    return 0;
}
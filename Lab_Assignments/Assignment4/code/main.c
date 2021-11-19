// coding: utf-8
// author: 18308013 ChenJiahao
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myLexer.h"
#include "myParser.h"
#include "mySemantics.h"

char SourceFilePath[150];
int main()
{
    printf("-----------------------------------------\n");
    printf("|   TINY+ v1.0 :: 18308013 ChenJiahao   |\n");
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
    FILE* tfp1 = fopen("output_token.txt", "w+");
    FILE* tfp2 = fopen("output_tree.txt", "w+");
    FILE* tfp3 = fopen("output_code.txt", "w+");
    FILE* tfp4 = fopen("output_table.txt", "w+");
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
        {
            fprintToken(tp, tfp1);
            fprintf(tfp1, "\n");
        }
    }
    fclose(tfp1);
    MaxTokenNum = TokenIndex;
    tpArray = (struct token**)realloc(tpArray, MaxTokenNum * sizeof(struct token*));
    TreeNode* root;
    if (isFinish)
    {
        printf("\n[INFO] Result has been generated in output_token.txt.\n");
        printf("\n[INFO] Lexical analysis finished.\n");
        printf("\n[INFO] Parser analysis: \n\n");
        root = analyseProgram(tpArray, MaxTokenNum);
        if (root == NULL)
        {
            printf("\n[INFO] Parser analysis interrupted.\n");
            system("pause");
            return 0;
        }
        int MaxDepth = getParserTreeMaxDepth(root);
        Bool* PrintFlagPtr = (Bool*)calloc(MaxDepth, sizeof(Bool));
        fprintParserTree(root, 1, PrintFlagPtr, tfp2);
        free(PrintFlagPtr);
        printf("\n[INFO] Result has been generated in output_tree.txt.\n");
        printf("\n[INFO] Parser analysis finished.\n");
    }

    printf("\n[INFO] Semantic analysis: \n\n");
    Symbol* header = buildNewSymbol(NULL, True);
    Symbol* ep = checkProgram(root, header, tfp3);
    if (ep!=NULL)
    {
        printf("\n[INFO] Code has been generated in output_code.txt.\n");
        fprintSymbolTable(header, 0, tfp4);
        printf("\n[INFO] Symbol Table has been generated in output_table.txt.\n");
        printf("\n[INFO] Finished.\n");
    }
    else
        printf("\n[INFO] Semantic analysis interrupted.\n");
    fclose(tfp3);
    fclose(tfp4);

    system("pause");
    return 0;
}
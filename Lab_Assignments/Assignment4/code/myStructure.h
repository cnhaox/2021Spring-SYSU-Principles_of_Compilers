#ifndef MYSTRUCTURE_H
#define MYSTRUCTURE_H

typedef int Bool;

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

// token结构
typedef struct token
{
    enum TokenType type; // token类型
    char* str;           // token内容
    int RowIndex;        // 起始行号
    int ColIndex;        // 起始列号
} token;

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
    NT_BOOL_MID_EXPR,
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

// 符号项类型
typedef enum SymbolType
{
    ST_FUNC,  // 函数
    ST_VAR,   // 变量
    ST_TABLE, // block符号表
    ST_HEAD,  // 符号表头部，无意义
    ST_ANY    // 未知
} SymbolType;

// 符号值类型
typedef enum ValueType
{
    VT_INT,
    VT_REAL,
    VT_BOOL,
    VT_STRING,
    VT_VOID,
    VT_ANY  // 未知
} ValueType;

// 符号项结构
typedef struct Symbol
{
    struct Symbol* prevSymPointer; // 指向当前符号表上一个Symbol的指针
    struct Symbol* nextSymPointer; // 指向当前符号表下一个Symbol的指针
    SymbolType symType;    // 符号类型
    token* tp;             // token指针
    struct Symbol* parentSymPointer; // 指向父符号表所属的Symbol的指针
    struct Symbol* childSymPointer;  // 指向属于自己的子符号表的头部Symbol的指针
    ValueType valType;     // 符号值类型
    Bool isVariable;       // 符号值是否可变
    Bool isParam;          // 是否为参数
    char* newName;         // 三地址码命名
    int address;           // 符号地址
    int size;              // 符号字节数
} Symbol;


#endif // !MYSTRUCTURE_H

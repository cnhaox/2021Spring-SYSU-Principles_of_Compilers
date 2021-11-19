#ifndef MYSTRUCTURE_H
#define MYSTRUCTURE_H

typedef int Bool;

// TOKEN����
typedef enum TokenType
{
    // �ؼ���
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
    NUMBER,     // ����
    ID,         // ������
    STRING,     // �ַ���
    // ����type
    ERROR,
    FINISH
} TokenType;

// DFA״̬����
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

// token�ṹ
typedef struct token
{
    enum TokenType type; // token����
    char* str;           // token����
    int RowIndex;        // ��ʼ�к�
    int ColIndex;        // ��ʼ�к�
} token;

// �ڵ�����
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

// �ڵ�
typedef struct TreeNode
{
    NodeType nodetype;// �ڵ�����
    struct TreeNode** ChildPtrList;// �ӽڵ�ָ�������ָ��
    int ChildCurrentIndex;// ��ǰ�ӽ������
    int ChildMaxNum;// �ӽ��ָ������֧�ֵ��������
    token* tp;// Ҷ�ӽڵ��Ӧ��TOKEN

} TreeNode;

// ����������
typedef enum SymbolType
{
    ST_FUNC,  // ����
    ST_VAR,   // ����
    ST_TABLE, // block���ű�
    ST_HEAD,  // ���ű�ͷ����������
    ST_ANY    // δ֪
} SymbolType;

// ����ֵ����
typedef enum ValueType
{
    VT_INT,
    VT_REAL,
    VT_BOOL,
    VT_STRING,
    VT_VOID,
    VT_ANY  // δ֪
} ValueType;

// ������ṹ
typedef struct Symbol
{
    struct Symbol* prevSymPointer; // ָ��ǰ���ű���һ��Symbol��ָ��
    struct Symbol* nextSymPointer; // ָ��ǰ���ű���һ��Symbol��ָ��
    SymbolType symType;    // ��������
    token* tp;             // tokenָ��
    struct Symbol* parentSymPointer; // ָ�򸸷��ű�������Symbol��ָ��
    struct Symbol* childSymPointer;  // ָ�������Լ����ӷ��ű��ͷ��Symbol��ָ��
    ValueType valType;     // ����ֵ����
    Bool isVariable;       // ����ֵ�Ƿ�ɱ�
    Bool isParam;          // �Ƿ�Ϊ����
    char* newName;         // ����ַ������
    int address;           // ���ŵ�ַ
    int size;              // �����ֽ���
} Symbol;


#endif // !MYSTRUCTURE_H

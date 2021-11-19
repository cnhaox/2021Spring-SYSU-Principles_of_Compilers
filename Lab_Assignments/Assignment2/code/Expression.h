#include<string>
#include<stack>
#include<vector>
#include<stdio.h>

#define VAL   0
#define LB    1
#define RB    2
#define PLUS  3
#define MINUS 3
#define MUL   3
#define DIV   3
using namespace std;
struct node                            // 树节点
{
    node *left = nullptr;
    node *right = nullptr;
    string value;
};

class Expression
{
    private:
        int GetPriority(char c);       // 获取字符c的优先级
        int GetPriority(string str);   // 获取字符串str的优先级
        void SplitExpression();        // 对表达式字符串expression划分
        bool GetPostfixExpression();   // 检测表达式合法性并转换成逆波兰表达式
        bool JudgeOperator(string str);// 判断str是否为运算符
    public:
        string expression;             // 算数表达式
        vector<string> expVec;         // 划分后的算数表达式
        vector<string> posexpVec;      // 逆波兰表达式
        bool isLegal;                  // 表达式是否合法
        Expression(string input)
        {
            expression=input;
        }
        vector<string> transfer()      // 算数表达式->逆波兰表达式
        {
            SplitExpression();
            if (GetPostfixExpression())
                printf("Sussecfully Done!\n");
            else
                printf("Error: Incorrect Format!\n");
            return posexpVec;
        }
        node *BuildTree();             // 获取语法树树根
};

int Expression::GetPriority(char c)
{// 判断字符c是否是操作符并返回优先级
    if (c=='(')
        return 1;
    else if (c=='+'||c=='-')
        return 2;
    else if (c=='*'||c=='/')
        return 3;
    else if (c==')')
        return 4;
    else if ((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z'))
        return 0;
    else if (c==' '||c=='\n'||c=='\t')
        return -1;
    else
        return -2;
}

int Expression::GetPriority(string str)
{
    if (str.length()<=0)
    return -1;
    char c=str[0];
    return GetPriority(c);
}

void Expression::SplitExpression()
{// 按照操作数和操作符的形式划分算数表达式
    for (int i=0; i<expression.length(); i++)
    {
        int priority = GetPriority(expression[i]);
        // 若为空白字符
        if (priority==-1)
            continue;
        // 若为操作数
        else if (priority == 0)
        {
            string temp;
            // 获取整个操作数
            while (i<expression.length() && GetPriority(expression[i])==0)
            {
                temp.push_back(expression[i]);
                i++;
            }
            expVec.push_back(temp);
            i--;
        }
        // 若为操作符
        else
        {
            string temp;
            temp.push_back(expression[i]);
            expVec.push_back(temp);
        }
    }
}

bool Expression::GetPostfixExpression()
{// 将expVec转换为后缀表达式vector
    stack<string> s;
    isLegal = true;
    int preType;
    for (int i=0; i<expVec.size(); i++)
    {
        int len = expVec[i].length();
        // 若长度为0，不合法
        if (len==0)
        {
            isLegal = false;
            break;
        }
        // 若长度大于1，则为操作数
        else if (len>1)
        {   
            // 若上一个也为操作数，不合法
            if (i>=1 && preType==VAL)
            {
                isLegal = false;
                break;
            }
            else
                preType = VAL;
            bool flag = true;
            // 判断操作数是否为数字开头的变量，若是则不合法
            if (expVec[i][0]>='0'&&expVec[i][0]<='9')
            {
                for (int j=1; j<len; j++)
                {
                    if (expVec[i][j]>='0'&&expVec[i][j]<='9')
                        continue;
                    else
                    {
                        flag=false;
                        break;
                    }
                }
            }
            // 合法，加入队列
            if (flag)
                posexpVec.push_back(expVec[i]);
            // 不合法，over
            else
            {
                isLegal=false;
                break;
            }
        }
        else
        {
            int prior = GetPriority(expVec[i][0]);
            // 若为操作符'+','-','*','/'且在最后一位，不合法
            if (prior>0 && prior<4 && i==expVec.size()-1)
            {
                isLegal = false;
                break;
            }
            // 若为操作数
            if (prior==0)
            {
                // 若上一个也是操作数，不合法
                if (i>=1 && preType==VAL)
                {
                    isLegal = false;
                    break;
                }
                preType = VAL;
                posexpVec.push_back(expVec[i]);
            }
            // 若为'('
            else if (prior==1)
            {
                // 若上一个为操作数或')'，不合法
                if (i>=1 && (preType==VAL||preType==RB))
                {
                    isLegal = false;
                    break;
                }
                preType = LB;
                s.push(expVec[i]);
            }
            // 若为'+'或'-'
            else if (prior==2)
            {
                // 若为操作数的正/负号，添0
                if (i==0 || (i>0&&preType==LB))
                    posexpVec.push_back("0");
                // 若上一个为'+'、'-'、'*'或'/'，不合法
                else if (preType==PLUS||preType==MINUS||preType==MUL||preType==DIV)
                {
                    isLegal = false;
                    break;
                }
                if (expVec[i][0]=='+')
                    preType = PLUS;
                else if (expVec[i][0]=='-')
                    preType = MINUS;
                // 弹出优先级大的操作符，再压栈
                while (!s.empty() && GetPriority(s.top())>=GetPriority(expVec[i]))
                {
                    posexpVec.push_back(s.top());
                    s.pop();
                }
                s.push(expVec[i]);
            }
            // 若为'*'或'-'
            else if (prior==3)
            {
                // 若在第一位出现，或者前一位不是操作数或')'，不合法
                if (i==0 || (preType!=VAL&&preType!=RB))
                {
                    isLegal = false;
                    break;
                }
                if (expVec[i][0]=='*')
                    preType = MUL;
                else if (expVec[i][0]=='/')
                    preType = DIV;
                // 弹出优先级大的操作符，再压栈
                while (!s.empty() && GetPriority(s.top())>=GetPriority(expVec[i]))
                {
                    posexpVec.push_back(s.top());
                    s.pop();
                }
                s.push(expVec[i]);
            }
            // 若为')'
            else if (prior==4)
            {
                // 若前一位不是操作数，不合法
                if (preType!=VAL&&preType!=RB)
                {
                    isLegal = false;
                    break;
                }
                preType = RB;
                bool isFinish = false;
                // 不断弹栈直到'('
                while (!s.empty())
                {
                    if (s.top()=="(")
                    {
                        s.pop();
                        isFinish = true;
                        break;
                    }
                    posexpVec.push_back(s.top());
                    s.pop();
                }
                // 栈中没有匹配的'('，不合法
                if (!isFinish)
                {
                    isLegal = false;
                    break;
                }
            }
            // 出若为其他符号，不合法
            else
            {
                isLegal = false;
                break;
            }
        }
    }
    // 清空栈
    while (isLegal&&!s.empty())
    {
        // 有多余'('时不合法
        if (s.top()=="(")
            isLegal = false;
        posexpVec.push_back(s.top());
        s.pop();
    }
    // 不合法时清空逆波兰表达式
    if (!isLegal)
        posexpVec.clear();
    return isLegal;
}

bool Expression::JudgeOperator(string str)
{// 判断str是否为操作符
    if (str.length()>1 || str.length()==0)
        return false;
    if (GetPriority(str)<=0)
        return false;
    return true;
}

node* Expression::BuildTree()
{// 将后缀表达式vec转换为语法树
    SplitExpression();
    if (!GetPostfixExpression())
    {
        printf("Error: Incorrect Format!\n");
        return nullptr;
    }
    stack<node*> s;
    for (int i=0; i<posexpVec.size(); i++)
    {
        node *np = new node;
        np->value=posexpVec[i];
        // 如果为操作符
        if (JudgeOperator(np->value))
        {
            np->right = s.top();
            s.pop();
            np->left = s.top();
            s.pop();
        }
        s.push(np);
    }
    return s.top();
}


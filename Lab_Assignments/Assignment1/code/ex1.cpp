#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<stack>
#include<vector>
using namespace std;

int GetPriority(char c);// 判断字符c是否是操作符并返回优先级
vector<string> GetPostfixExpression(string exp);// 将表达式字符串exp转换为后缀表达式vector

int main()
{
    cout << "Please input the expression: ";
    string exp;
    cin >> exp;
    vector<string> output=GetPostfixExpression(exp);
    cout << "output: ";
    for (int i=0; i<output.size(); i++)
    {
        if (i>0)
        cout << ",";
        cout << output[i];
    }
    cout << endl;
    system("pause");
    return 0;
}

int GetPriority(char c)
{// 判断字符c是否是操作符并返回优先级
    switch (c)
    {
        case '(':
            return 1;
            break;
        case '+':
        case '-':
            return 2;
            break;
        case '*':
        case '/':
            return 3;
            break;
        case ')':
            return 4;
            break;
        default:// 字符c为非操作符时返回-1
            return -1;
            break;
    }
}

vector<string> GetPostfixExpression(string exp)
{// 将表达式字符串exp转换为后缀表达式vector
    vector<string> output;
    string temp;
    stack<char> s;
    for (int i=0; i<exp.length(); i++)
    {
        if (exp[i]==' ')
            continue;
        // 当前字符为非操作符时
        if (GetPriority(exp[i]) == -1)
        {
            // 获取操作数
            string temp;
            while(i<exp.length() && GetPriority(exp[i]) == -1)
            {
                temp.push_back(exp[i]);
                i++;
            }
            // 操作数压栈
            output.push_back(temp);
            i-=1;
        }
        // 当前字符为操作符时
        else
        {
            // 栈空，压栈
            if (s.empty())
                s.push(exp[i]);
            //操作符为'('，压栈
            else if (exp[i]=='(')
                s.push(exp[i]);
            // 操作符为')'，弹栈
            else if (exp[i]==')')
            {
                while (!s.empty())
                {
                    if (s.top()=='(')
                    {
                        // 栈顶为'('时结束
                        s.pop();
                        break;
                    }
                    string temp(1, s.top());
                    output.push_back(temp);
                    s.pop();
                }
            }
            else
            {
                // 当前操作符优先级大于栈顶元素，压栈
                if (GetPriority(s.top()) < GetPriority(exp[i]))
                    s.push(exp[i]);
                // 否则弹栈
                else
                {
                    while (!s.empty() && GetPriority(s.top())>=GetPriority(exp[i]))
                    {
                        string temp(1, s.top());
                        output.push_back(temp);
                        s.pop();
                    }
                    s.push(exp[i]);
                }
            }
        }
    }
    // 扫描字符串结束，清空栈
    while (!s.empty())
    {
        string temp(1, s.top());
        output.push_back(temp);
        s.pop();
    }
    return output;
}

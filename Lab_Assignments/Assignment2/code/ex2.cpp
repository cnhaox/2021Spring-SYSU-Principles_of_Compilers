#include<iostream>
#include<stdio.h>
#include<string>
#include<stack>
#include<vector>
#include "Expression.h"
using namespace std;
void GetTreePrintFormat(node *root, vector<string>& output, int depth)
{// 打印语法树显示
    if (root==nullptr)
        return;
    GetTreePrintFormat(root->left, output, depth+1);
    string space_str(root->value.length(), ' ');
    if (output.size()<=depth)
    {
        if (output.size()==0)
        {
            string temp;
            output.push_back(temp);
        }
        while (output.size()<=depth)
        {
            string temp(output[output.size()-1].length(), ' ');
            output.push_back(temp);
        }
    }
    for (int i=0; i<output.size(); i++)
    {
        output[i].push_back(' ');
        if (i==depth)
            output[i]+=root->value;
        else
            output[i]+=space_str;
    }
    GetTreePrintFormat(root->right, output, depth+1);
}

int main()
{
    string input;
    cout << "Please input the expression: ";
    cin >> input;
    Expression exp(input);
    node *root=exp.BuildTree();
    if (root!=nullptr)
    {
        vector<string> output;
        GetTreePrintFormat(root, output, 0);
        cout << "--------------------------------------" << endl;
        for (int i=0; i<output.size(); i++)
            cout << output[i] << endl;
        cout << "--------------------------------------" << endl;
    }
    system("pause");
    return 0;
}

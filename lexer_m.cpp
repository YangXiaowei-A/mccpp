#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;

//保留字表 1 - 20，在matlab输入iskeyword可以查看
static char reserveWord[20][20] = {
    "break", "case", "catch", "classdef", "continue", "else",
    "elseif", "end", "for", "function", "global", "if",
    "otherwise", "parfor", "persistent", "return", "spmd", "switch",
    "try", "while"
};

// https://ww2.mathworks.cn/help/matlab/matlab_prog/matlab-operators-and-special-characters.html
// 界符，运算符，并没有包含所有的运算符，矩阵运算符不在之内 20 - 48
// 算术运算符：+ - * /   21 - 24
// 特殊符号：
// 赋值运算符：=    25
// 逗号：,  26
// 冒号：:  27
// 分号：； 28
// 圆括号：()   29 - 30
// 方括号：[]   31 - 32
// 花括号：{}   33 - 34
// 百分号: %    35
// 块注释(百分号加花括号)：%{ %}    
// 单引号:'' 要用反斜杠 36
// 双引号："" 也要用反斜杠  37
// 关系运算符：== ~= > >= < <= 38 - 43
// 逻辑运算符：& | && || ~ 44 - 48

static char operatorOrDelimiter[28][10] = {
   "+","-","*","/",
   "=",",",":",";","(",")","[","]","{","}","%","\'","\"",
   "==","~=",">",">=","<","<=",
   "&","|","&&","||","~"
};

static  char IDentifierTbl[1000][50] = {""};//标识符表

bool isDigit(char ch)//判断是否为数字
{
    if(ch>= '0' && ch <= '9')
        return true;
    return false;
}
bool isLetter(char ch)//判断是否为字母或下划线
{
    if((ch>= 'a' && ch<='z') || ( ch <= 'Z' && ch >= 'A')|| ch == '_')
        return true;
    return false;
}
int isReserve(char *s)//判断是否为保留字
{   
    for(int i=0;i<20;++i)
    {
        if(strcmp(reserveWord[i],s) == 0)
            return i+1;//返回种别码
    }
    return -1;    
}

void filter(char *s,int len)//源程序预处理，过滤注释和换行回车制表符
{
    char tmp[10000];
    int p = 0;
    for(int i=0;i<len;++i)
    {
        if(s[i] == '%')//单行注释
        {
            while(s[i]!='\n') ++i;//扫描到换行符为止
        }
        if(s[i] == '%' && s[i+1] == '{')//多行注释
        {
            i+=2;
            while(s[i] != '%' && s[i+1] != '}')
            {
                if(s[i] == '\0')
                {
                    cout<<"annotation error!"<<endl;
                    exit(0);
                }
                i++;
            }
            i+=2;
        }
        if(s[i] != '\n' && s[i] != '\t' && s[i] != '\v' && s[i] != '\r')
        {
            tmp[p] = s[i];
            ++p;
        }
    }
    tmp[p] = '\0';
    strcpy(s,tmp);
}

void scanner(int &syn,char * project,char * token,int &p)//扫描源程序，syn是种别码，token是当前扫描的单词，p为扫描位置索引
{   
    int count = 0;
    char ch;
    ch = project[p];
    while(ch == ' ')//去掉空格
    {
        ++p;
        ch = project[p];
    }
    for(int i=0;i<20;i++)//清空token
    {
        token[i] = '\0';
    }
    if(isLetter(project[p]))
    {//以字母开头
        token[count++] = project[p++];
        while(isLetter(project[p])||isDigit(project[p]))//后面是字母或数字
        {
            token[count++] = project[p++];
        }
        token[count] = '\0';
        syn = isReserve(token);//查表找到种别码
        if(syn == -1)
        {//若不是保留字则是标识符
            syn = 100;//标识符种别码
        }
        return;
    }
    else if(isDigit(project[p]))
    {//以数字开头
       token[count++] = project[p++]; 
       while(isDigit(project[p]))//后面是数字
        {
            token[count++] = project[p++];
        }
        token[count] = '\0';
        syn = 99;
        return;
    }
    else if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ',' || ch == ':' || ch == ';'
            || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '%'
            || ch == '\'' || ch == '\"')
    {
        token[count++] = project[p++];
        token[count] = '\0';
        for(int i=0;i<28;++i)
        {
            if(strcmp(operatorOrDelimiter[i],token) == 0)
            {
                syn = 21+i;
                break;
            }
        }
        return;
    }
    else if(ch == '<')
    {//可能是< <=
        ++p;
        if(project[p] == '=')//<=
        {
            syn = 43;
        }
        else//<
        {
            --p;
            syn = 42;
        }
        ++p;
        return;
    }
    else if(ch == '>')
    {//可能是> >= >>
        ++p;
        if(project[p] == '=')//>=
        {
            syn = 41;
        }
        else//>
        {
            --p;
            syn = 40;
        }
        ++p;
        return;
    }
    else if(ch == '=')
    {//可能是= ==
        ++p;
        if(project[p] == '=')//==
        {
            syn = 38;
        }
        else
        {//=
            --p;
            syn = 25;
        }
        ++p;
        return;
    }
    else if(ch == '~')
    {//可能是! !=
        ++p;
        if(project[p] == '=')//~=
        {
            syn = 39;
        }
        else //~
        {
            --p;
            syn = 48;
        }
        ++p;
        return;
    }
    else if(ch == '&')
    {//可能是& &&
        ++p;
        if(project[p] == '&')//&&
        {
            syn = 46;
        }
        else
        {//&
            --p;
            syn = 44; 
        }
        ++p;
        return;
    }
    else if(ch == '|')
    {//可能是| ||
        ++p;
        if(project[p] == '|')//||
        {
            syn = 45;
        }
        else
        {
            --p;
            syn = 47;
        }
        ++p;
        return;
    }
    else if(ch == '\0')//文件结束
    {
        syn = 0;
    }
    else
    {
        cout<<"wrong letter:"<<ch<<endl;
        exit(0);
    }
}

int main()
{
    cout << "hello mccpp" << endl;
    //打开一个文件，读取源程序
    char project[10000];
    char token[20] = {0};
    int syn = -1;
    int p = 0; //程序位置索引
    ifstream in("test_m.txt");
    ofstream out("out_m.txt");
    if(!in.is_open())
    {
        cout << "error opening file!"<<endl;
        exit(0);
    }
    while(!in.eof())
    {
        in.get(project[p++]);   //把文本文件的字符流存储到project数组
    }
    project[p++] = '\0';
    in.close();
    cout<<"源程序为:\n"<<project<<endl;
    filter(project,p);
    cout<<"过滤后的源程序为:\n"<<project<<endl;
    p = 0;
    while(syn != 0)//开始扫描
    {
        scanner(syn,project,token,p);
        if(syn == 100)//标识符(变量)
        {
            for(int i = 0;i<1000;i++)
            {//插入标识符表
                if(strcmp(IDentifierTbl[i],token) == 0)
                {//已存在表中
                    break;
                }
                else if(strcmp(IDentifierTbl[i],"") == 0)
                {
                    strcpy(IDentifierTbl[i],token);
                    break;
                }
            }
            cout<<"标识符:"<<token<<endl;
            out<<"标识符:"<<token<<endl;
        }
        else if(syn == 99)//常数
        {
            cout<<"常数:"<<token<<endl;
            out<<"常数:"<<token<<endl;
        }
        else if(syn <= 20 && syn >= 1)//保留字
        {
            cout<<reserveWord[syn - 1]<<":"<<syn<<endl;
            out<<reserveWord[syn - 1]<<":"<<syn<<endl;
        }
        else if(syn >= 21 && syn <= 48)//运算符或界符
        {
            cout<<operatorOrDelimiter[syn - 21]<<":"<<syn<<endl;
            out<<operatorOrDelimiter[syn - 21]<<":"<<syn<<endl;
        }
    }
    out.close();
}
#pragma once
// ========== 删除以下DLL导出/导入宏定义 ==========
// #ifdef Data_initial_EXPORTS
// #define EXP extern "C" __declspec(dllexport)
// #else
// #define EXP extern "C" __declspec(dllimport)
// #endif

#include "Universal_headers.h"
#include "User_Define_Using.h"

namespace data_initial {
using namespace std;

// ========== 移除所有EXP标记，恢复普通函数声明 ==========
inline DWORD Hash4(const string&);//12位字符串哈希
bool initial_readers(DWORD, DWORD, bool, char*);//多线程读取文件
void reader(fstream&, DWORD, DWORD, DWORD);//数据读取子线程
inline DWORD Write_Record(fstream&, DWORD, string&, DWORD);
inline void Create_article(string, DWORD, vector<string>&, DWORD, DWORD);
inline void do_writer();//文件写入逻辑
void writer();//数据写入子线程
inline void push_wz(string, DWORD);
inline void push_year(string, DWORD);
inline void push_author(string, DWORD);
inline bool gotchar(fstream&, DWORD, char&);
inline LPCWSTR stringtolstr(string);
template<typename T> inline void sort(vector<T>&, int, int);
class out_string; // 输出数据存储类
extern std::atomic<long long> total_num;
}

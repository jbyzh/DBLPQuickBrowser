#pragma once

#include "Universal_headers.h"
#include "User_Define_Using.h"

namespace data_initial {
using namespace std;

inline DWORD Hash4(const string&);
bool initial_readers(DWORD, DWORD, bool, char*);
void reader(fstream&, DWORD, DWORD, DWORD);
inline DWORD Write_Record(fstream&, DWORD, string&, DWORD);
inline void Create_article(string, DWORD, vector<string>&, DWORD, DWORD);
inline void do_writer();
void writer();
inline void push_wz(string, DWORD);
inline void push_year(string, DWORD);
inline void push_author(string, DWORD);
inline bool gotchar(fstream&, DWORD, char&);
inline LPCWSTR stringtolstr(string);
template<typename T> inline void sort(vector<T>&, int, int);

class out_string;
extern std::atomic<long long> total_num;
}

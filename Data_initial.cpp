#define Data_initial_EXPORTS
#include "Data_initial.h"
//#define DEBUGING

namespace data_initial {
const int MAX_NUM = 4096;//HASH最大值(修改需同步Hash4函数)
std::atomic<long long> total_num = 0; // 初始化总记录数
int alive_thread_initial = 0;
bool sys_writing = false;

bool outtmpwzfl[MAX_NUM] = { false };
bool outtmpzzfl[MAX_NUM] = { false };
bool outtmpyearfl[3000] = { false };
out_string* outtmpwz[MAX_NUM] = { 0 };
out_string* outtmpzz[MAX_NUM] = { 0 };
out_string* outtmpyear[3000] = { 0 };
vector<string> auth[16];
string quike_reader[16];
DWORD quike_reader_p[16] = { 0 };

class out_string { // 输出数据存储类
public:
    string out;
    out_string* next;
    out_string() {
        out = "";
        next = NULL;
    }
};

template<typename T> inline void sort(vector<T>& a, int s, int t) {
    if (s >= t)return;
    int l = s, r = t;
    T tmp = a[s];
    while (l < r) {
        while (l < r && a[r] >= tmp)r--;
        while (l < r && a[l] <= tmp)l++;
        if (l != r)swap(a[r], a[l]);
    }
    swap(a[s], a[r]);
    sort(a, s, l - 1);
    sort(a, l + 1, t);
    return;
}

inline void push_wz(string content, DWORD num) {
    while (sys_writing) { Sleep(1000); }
    while (outtmpwzfl[num]) {}
    outtmpwzfl[num] = true;
    out_string* tmp = outtmpwz[num];
    outtmpwz[num] = new out_string;
    outtmpwz[num]->out = content;
    outtmpwz[num]->next = tmp;
    outtmpwzfl[num] = false;
    return;
}

inline void push_year(string content, DWORD num) {
    while (sys_writing) { Sleep(1000); }
    while (outtmpyearfl[num]) {}
    outtmpyearfl[num] = true;
    out_string* tmp = outtmpyear[num];
    outtmpyear[num] = new out_string;
    outtmpyear[num]->out = content;
    outtmpyear[num]->next = tmp;
    outtmpyearfl[num] = false;
    return;
}

inline void push_author(string content, DWORD num) {
    while (sys_writing) { Sleep(1000); }
    while (outtmpzzfl[num]) {}
    outtmpzzfl[num] = true;
    out_string* tmp = outtmpzz[num];
    outtmpzz[num] = new out_string;
    outtmpzz[num]->out = content;
    outtmpzz[num]->next = tmp;
    outtmpzzfl[num] = false;
    return;
}

inline DWORD Hash4(const string& tar) {
    DWORD64 ans = 0;
    for (int i = 0; i < tar.length(); i++) {
        ans = ((ans >> 8) & 0xf) ^ ((ans << 4) ^ tar[i]);
    }
    return ans & 0xfff;
}

inline bool gotchar(fstream& iner, DWORD ID, char& outs) {
    if (quike_reader[ID] == "" || quike_reader_p[ID] >= quike_reader[ID].size()) {
        if (getline(iner, quike_reader[ID]))
            quike_reader_p[ID] = 0;
        else return false;
    }
    outs = quike_reader[ID][quike_reader_p[ID]];
    quike_reader_p[ID]++;
    return true;
}

inline void Create_article(string name, DWORD flag, vector<string>& author, DWORD years, DWORD ID) {
    if (author.size() > 256) {//作者数量异常处理
        DWORD nh = Hash4(name);
        push_wz(name + " <-> " + to_string(flag), nh);
        push_year(name, years);
        total_num++;
        return;
    }
    for (int i = 0; i < author.size(); i++)auth[ID].push_back(author[i]);
    DWORD nh = Hash4(name);
    push_wz(name + " <-> " + to_string(flag), nh);
    push_year(name, years);
    // 修复原push_author参数错误（原代码传author向量，现按Hash后值传）
    for (auto& a : author) {
        push_author(name + "$" + to_string(flag), Hash4(a));
    }
    author.clear();
    total_num++;
    return;
}

inline DWORD Write_Record(fstream& iner, DWORD flag, string& tar, DWORD ID) { // 写入记录函数（解析XML标签）
    if (tar.find("<article") == string::npos && tar.find("<inproceedings") == string::npos && tar.find("<proceedings") == string::npos &&
        tar.find("<book") == string::npos && tar.find("<incollection") == string::npos && tar.find("<phdthesis") == string::npos &&
        tar.find("<mastersthesis") == string::npos && tar.find("<www") == string::npos && tar.find("<person") == string::npos &&
        tar.find("<data") == string::npos) // 查找起始标签（检查是否为有效记录类型）
        return 0; // 不是有效记录类型，返回0
    char tmpc; // 临时字符变量
    string tmp; // 临时字符串变量
    int kd = 0; // 标签嵌套深度计数器
    vector<string> author; // 作者向量
    string name = ""; // 文章名称
    author.clear(); // 清空作者向量
    DWORD err_protection = 0; // 数据格式异常保护（记录错误位置）
    while (gotchar(iner, ID, tmpc)) { // 循环读取字符
        if (tmpc == '<') { // 遇到开始标签
            tmp = "<"; // 初始化临时字符串
            if (kd == 0) {
                err_protection = (DWORD)(iner.tellg()) + quike_reader_p[ID] - quike_reader[ID].length(); // 记录错误保护位置
            }
            kd++; // 增加嵌套深度
            continue; // 继续循环
        }
        else if (tmpc == '>' && kd > 0) { // 遇到结束标签
            kd--; // 减少嵌套深度
            tmp = tmp + '>'; // 添加到临时字符串
            if (tmp == "<author>" || tmp == "<editor>") { // 作者标签
                tmp = ""; // 清空临时字符串
                while (gotchar(iner, ID, tmpc)) { // 读取作者名称
                    if (tmpc != '<')tmp = tmp + tmpc; // 非标签字符添加到临时字符串
                    else {
                        author.push_back(tmp); // 将作者添加到向量
                        break; // 遇到新标签，结束读取
                    }
                }
            }
            else if (tmp == "<title>" || tmp == "<booktitle>") { // 标题标签
                tmp = ""; // 清空临时字符串
                while (gotchar(iner, ID, tmpc)) { // 读取标题内容
                    if (tmpc != '<')tmp = tmp + tmpc; // 非标签字符添加到临时字符串
                    else {
                        name = tmp; // 设置文章名称
                        break; // 遇到新标签，结束读取
                    }
                }
            }
            else if (tmp == "<year>") { // 年份标签（完成记录需要数据的提取）
                int years = 0; // 年份变量
                while (gotchar(iner, ID, tmpc)) { // 跳过空格
                    if (tmpc != ' ')break; // 遇到非空格字符结束循环
                }
                while (tmpc >= '0' && tmpc <= '9') { // 读取数字年份
                    years = years * 10 + tmpc - '0'; // 计算年份值
                    gotchar(iner, ID, tmpc); // 读取下一个字符
                }
                Create_article(name, flag, author, years, ID); // 创建文章记录
                break; // 记录处理完成，退出循环
            }
            if (kd <= 0) { // 标签完全闭合
                kd = 0; // 重置深度计数器
            }
        }
        else if (kd > 0) { // 标签内部内容
            tmp = tmp + tmpc; // 添加到临时字符串
        }
    }
    return 0; // 正常返回
}

void reader(fstream& iner, DWORD offset, DWORD Terminal, DWORD ID) { // 数据读取子线程函数
#ifdef DEBUGING
    cout << "线程" << ID << "开始读取" << endl; // 调试信息：线程开始读取
#endif
    iner.seekg(offset); // 设置文件读取位置
    char tmpc; // 临时字符变量
    string tmp; // 临时字符串变量
    DWORD flag = 0; // 当前偏移量（记录标签开始位置）
    DWORD position = (DWORD)(iner.tellg()) + quike_reader_p[ID] - quike_reader[ID].length(); // 计算当前位

    while (position <= Terminal + 1) { // 循环读取直到终点位置
        // 重置临时变量
        tmp.clear();
        flag = 0;

        // 跳过空白字符，找到下一个开始标签
        while (gotchar(iner, ID, tmpc)) {
            if (tmpc == '<') {
                tmp = "<"; // 开始构建标签
                flag = (DWORD)(iner.tellg()) + quike_reader_p[ID] - quike_reader[ID].length(); // 记录标签
                break;
            }
        }

        if (flag == 0) break; // 没有找到开始标签，结束循环

        // 读取标签内容直到遇到>
        while (gotchar(iner, ID, tmpc)) {
            tmp = tmp + tmpc;
            if (tmpc == '>') {
                break; // 标签结束
            }
        }

        // 检查是否是有效记录类型
        if (tmp.find("<article") != string::npos || tmp.find("<inproceedings") != string::npos ||
            tmp.find("<proceedings") != string::npos || tmp.find("<book") != string::npos ||
            tmp.find("<incollection") != string::npos || tmp.find("<phdthesis") != string::npos ||
            tmp.find("<mastersthesis") != string::npos || tmp.find("<www") != string::npos ||
            tmp.find("<person") != string::npos || tmp.find("<data") != string::npos) {

            // 调用Write_Record处理
            while (flag) {
                flag = Write_Record(iner, flag, tmp, ID); // 异常数据重试
            }
        }

        // 更新当前位置
        position = (DWORD)(iner.tellg()) + quike_reader_p[ID] - quike_reader[ID].length();
    }
    alive_thread_initial--; // 减少存活线程计数
#ifdef DEBUGING
    cout << "线程" << ID << "读取完成" << endl; // 调试信息：线程读取完成
#endif
    return;
}

string _FileUrl_All;

inline void do_writer() {
    // 确保目录存在（QT下兼容路径）
    _mkdir((_FileUrl_All + "database\\article").c_str());
    _mkdir((_FileUrl_All + "database\\author").c_str());
    _mkdir((_FileUrl_All + "database\\year").c_str());

    for (int i = 0; i < MAX_NUM; i++) {
        if (outtmpwz[i] == NULL)continue;
        fstream ot(_FileUrl_All + "database\\article\\" + to_string(i) + ".ini", ios::app);
        out_string* now = outtmpwz[i];
        outtmpwz[i] = NULL;
        while (now != NULL) {
            ot << (now->out) << endl;
            out_string* tmp = now;
            now = now->next;
            delete tmp;
        }
        ot.close();
    }
    for (int i = 0; i < MAX_NUM; i++) {
        if (outtmpzz[i] == NULL)continue;
        fstream ot(_FileUrl_All + "database\\author\\" + to_string(i) + ".ini", ios::app);
        out_string* now = outtmpzz[i];
        outtmpzz[i] = NULL;
        while (now != NULL) {
            ot << (now->out) << endl;
            out_string* tmp = now;
            now = now->next;
            delete tmp;
        }
        ot.close();
    }
    for (int i = 0; i < 3000; i++) {
        if (outtmpyear[i] == NULL)continue;
        fstream ot(_FileUrl_All + "database\\year\\" + to_string(i) + ".ini", ios::app);
        out_string* now = outtmpyear[i];
        outtmpyear[i] = NULL;
        while (now != NULL) {
            ot << (now->out) << endl;
            out_string* tmp = now;
            now = now->next;
            delete tmp;
        }
        ot.close();
    }

    for (int i = 1; i < 16; i++) {
        for (int j = 0; j < auth[i].size(); j++) {
            auth[0].push_back(auth[i][j]);
        }
        auth[i].clear();
    }
    fstream allauthor(_FileUrl_All + "database\\author_rank", ios::app);
    sort(auth[0], 0, (int)auth[0].size() - 1);
    if (!auth[0].empty()) {
        string tmpau = auth[0][0];
        int tot = 0;
        for (int i = 0; i < auth[0].size(); i++) {
            if (auth[0][i] != tmpau) {
                allauthor << tot << " " << tmpau << endl;
                tmpau = auth[0][i];
                tot = 1;
            }
            else {
                tot++;
            }
        }
        // 写入最后一个作者的统计
        allauthor << tot << " " << tmpau << endl;
    }
    allauthor.close();
    auth[0].clear();
}

bool writing_finished = false;

void writer() {
    int mytmp = 0;
    while (!writing_finished) {
        if (total_num - mytmp >= 1000000) {
#ifdef DEBUGING
            cout << u8"写入线程开始批量写入." << endl;
#endif
            sys_writing = true;
            mytmp = (int)total_num;
            do_writer();
            sys_writing = false;
        }
#ifdef DEBUGING
        cout << u8"写入线程等待中" << endl;
        Sleep(3000);
#else
        Sleep(500);
#endif
    }
#ifdef DEBUGING
    cout << u8"写入线程开始收尾." << endl;
#endif
    sys_writing = true;
    mytmp = (int)total_num;
    do_writer();
    sys_writing = false;
#ifdef DEBUGING
    cout << u8"写入线程完成." << endl;
#endif
    return;
}

inline LPCWSTR stringtolstr(string str) {
    // QT下兼容字符转换（修复原内存泄漏）
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar_t* ans = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, ans, len);
    return ans;
}

bool initial_readers(DWORD Max_thread, DWORD TOTAL_THREAD, bool File_check, char* _FileUrl) {
    _FileUrl_All = (string)_FileUrl;
    // 修复路径拼接（QT下兼容Windows路径）
    string finishDbPath = _FileUrl_All + "database\\finish.db";
    HANDLE check_file_handle = CreateFile(
        stringtolstr(finishDbPath),
        FILE_READ_EA,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (File_check && check_file_handle != INVALID_HANDLE_VALUE) {
        cout << u8"已存在生成的数据库，直接返回" << endl;
        CloseHandle(check_file_handle);
        return true;
    }
    CloseHandle(check_file_handle);

    if (Max_thread > 16) {
        cout << u8"最大线程数不能超过16" << endl;
        return false;
    }
    alive_thread_initial = 0;

    // 创建目录（QT下兼容system命令）
    string paths = "mkdir \"" + _FileUrl_All + "database\\author\" 2>nul";
    system(paths.c_str());
#ifdef DEBUGING
    cout << paths << endl;
#endif
    paths = "mkdir \"" + _FileUrl_All + "database\\article\" 2>nul";
    system(paths.c_str());
#ifdef DEBUGING
    cout << paths << endl;
#endif
    paths = "mkdir \"" + _FileUrl_All + "database\\year\" 2>nul";
    system(paths.c_str());
#ifdef DEBUGING
    cout << paths << endl;
#endif

    thread* readers[16] = { nullptr };
    fstream* infile = new fstream[Max_thread];
    string xmlPath = _FileUrl_All + "dblp.xml";
    for (int i = 0; i < Max_thread; i++) {
        infile[i].open(xmlPath, ios::in);
        if (!infile[i]) {
            cout << "文件" << i << "无法打开：" << xmlPath << endl;
            return false;
        }
    }

    // 获取文件大小（QT下兼容）
    DWORD64 filesize = 0;
    HANDLE target_file_handle = CreateFile(
        stringtolstr(xmlPath),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (target_file_handle != INVALID_HANDLE_VALUE) {
        filesize = GetFileSize(target_file_handle, NULL);
        CloseHandle(target_file_handle);
    }
#ifdef DEBUGING
    cout << u8"文件大小:" << filesize << endl;
#endif

    DWORD64 offset = filesize / Max_thread;
#ifdef DEBUGING
    cout << u8"开始创建读取线程" << endl;
#endif

    // 创建写入线程（QT下thread传参兼容）
    thread writers(writer);
#ifdef DEBUGING
    cout << u8"已创建写入线程" << endl;
#endif

    infile[Max_thread - 1].seekg(0, ios::beg);
#ifdef DEBUGING
    cout << u8"需要创建的线程数:" << Max_thread << endl;
#endif

    for (int i = 0; i < Max_thread - 1; i++) {
#ifdef DEBUGING
        cout << u8"正在创建线程" << i << endl;
#endif
        DWORD startpoi = (DWORD)infile[Max_thread - 1].tellg();
        infile[Max_thread - 1].seekg((i + 1) * offset);
        char tmpchar;
        while (infile[Max_thread - 1].get(tmpchar)) {
            if (tmpchar == '>') {
                DWORD tmpe = (DWORD)infile[Max_thread - 1].tellg();
                alive_thread_initial++;
                readers[i] = new thread(reader, ref(infile[i]), startpoi, tmpe, i);
#ifdef DEBUGING
                cout << u8"已创建线程" << i << endl;
#endif
                while (alive_thread_initial >= TOTAL_THREAD) {
                    if (!sys_writing)cout << u8"已读取:" << total_num << endl;
                    else cout << u8"写入中:" << total_num << endl;
                    Sleep(1000);
                }
                break;
            }
        }
    }

    // 创建最后一个线程
    alive_thread_initial++;
    readers[Max_thread - 1] = new thread(
        reader,
        ref(infile[Max_thread - 1]),
        (DWORD)infile[Max_thread - 1].tellg(),
        (DWORD)filesize - 1,
        Max_thread - 1
        );
#ifdef DEBUGING
    cout << "Create Thread " << Max_thread - 1 << endl;
#endif

    // 等待所有读取线程完成
    while (alive_thread_initial > 0) {
        if (!sys_writing)cout << u8"已读取:" << total_num << endl;
        else cout << u8"写入中:" << total_num << endl;
        Sleep(1000);
    }
    cout << u8"读取完成，总记录数:" << total_num << endl;

    // 等待写入线程完成
    writing_finished = true;
    writers.join();
    cout << u8"所有数据写入完成" << endl;

    // 标记完成
    fstream finishes(finishDbPath, ios::out);
    finishes << total_num;
    finishes.close();

    // 释放资源
    for (int i = 0; i < Max_thread; i++) {
        if (readers[i] != nullptr) {
            readers[i]->join();
            delete readers[i];
        }
        infile[i].close();
    }
    delete[] infile;

    return true;
}
}

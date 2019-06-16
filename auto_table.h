#ifndef _AUTO_TABLE_H_
#define _AUTO_TABLE_H_

#include "common.h"
#include <vector>
#include <map>

using std::vector;
using std::map;

struct tagVarInfo {
    string strVarPre;       //变量前缀
    string strVarType;      //C++变量类型

    tagVarInfo() {}

    tagVarInfo(const tagVarInfo& other) {
        strVarPre = other.strVarPre;
        strVarType = other.strVarType;
    }

    tagVarInfo& operator = (const tagVarInfo& other) {
        if (this == &other)
            return *this;

        strVarPre = other.strVarPre;
        strVarType = other.strVarType;

        return *this;
    }
};

struct tagRow {
    //数据库列信息
    uint32 uiLength;
    string strType;
    string strRowName;
    //根据数据库列信息查找得到的C++信息
    tagVarInfo stVarInfo;
    tagRow() {
        uiLength = 0;
    }
    tagRow(const tagRow& other) {
        uiLength = other.uiLength;
        strType = other.strType;
        strRowName = other.strRowName;
        stVarInfo = other.stVarInfo;
    }
    tagRow& operator = (const tagRow& other) {
        if (this == &other)
            return *this;

        uiLength = other.uiLength;
        strType = other.strType;
        strRowName = other.strRowName;
        stVarInfo = other.stVarInfo;

        return *this;
    }
};

struct tagTable {
    string strTable;
    vector<tagRow> vecRow;
};


class AutoTable {
public:
    AutoTable() {}
    ~AutoTable() {}
    //strInPut：输入sql建表语句；strOutPutInc：输出数据库操作自动化类头文件;strOutPutSrc：输出数据库操作自动化类源文件,
    void Analysis(const char* szFile);

private:
    void Init();
    void AddVarInfo(const char* szRowType, const char* szVarPre, const char* szVarType);
    //过滤每行的所有tab键，并且过滤行首空格键
    int32 SplitStringByEnter(const string& strInput, vector<string>& vecContent);
    int32 GetTableFromVec(vector<string>& vecContent, vector<tagTable>& vecTable);
    void SplitStringByTab(string& strLine); //过滤tab键
    int32 AnalysisTable(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);

    void CreateDestruct(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateDefaultConstructor(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateCopyConstructor(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateGetRowName(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateTableName(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateSetRowValue(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateGetRowValue(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateHas(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateClearHas(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateOperatorEqual(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateGetCount(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateSerialize(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateClassVariables(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);
    void CreateIsValid(const tagTable& stTable, string& strOutPutInc, string& strOutPutSrc);

private:
    map<string, tagVarInfo> m_mapVarInfo;
};

#endif

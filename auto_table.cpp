#include "auto_table.h"

#include <string.h>

#include <algorithm>

#include "file.h"

const char szString[] = "string";
const uint32 FUNCTION_LENGTH = 8192;

void AutoTable::Analysis(const char *szFile) {
    if (NULL == szFile) {
        ERRORLOG("in put empty");
        return;
    }

    int iRet;
    string strSql, strInc, strSrc;

    if (0 != (iRet = File::ReadFileToString(szFile, &strSql))) {
        ERRORLOG("Invalid argument.err msg=%s. %s\n", strerror(iRet), szFile);
        return;
    }

    Init();

    vector<string> vecContent;
    if (SUCCESS != SplitStringByEnter(strSql, vecContent)) {
        return;
    }

    vector<tagTable> vecTable;
    if (SUCCESS != GetTableFromVec(vecContent, vecTable)) {
        return;
    }

    string strIncMacro;
    for (vector<tagTable>::iterator iter = vecTable.begin(); iter != vecTable.end(); iter++) {
        strInc = "";
        strSrc = "";
        strIncMacro = iter->strTable;

        transform(strIncMacro.begin(), strIncMacro.end(), strIncMacro.begin(), toupper);

        strInc.append("#ifndef " + strIncMacro + "_H_" + "\n#define " + strIncMacro + "_H_\n\n");

        strInc +=
            "#include <string>\n\n"
            "using std::string;\n\n"
            "#ifndef tb_int8\n#define tb_int8    char\n#endif\n\n"
            "#ifndef tb_uint8\n#define tb_uint8    unsigned char\n#endif\n\n"
            "#ifndef tb_int16\n#define tb_int16    short\n#endif\n\n"
            "#ifndef tb_uint16\n#define tb_uint16    unsigned short\n#endif\n\n"
            "#ifndef tb_int32\n#define tb_int32    int\n#endif\n\n"
            "#ifndef tb_uint32\n#define tb_uint32    unsigned int\n#endif\n\n"
            "#ifndef tb_int64\n#define tb_int64    long long\n#endif\n\n"
            "#ifndef tb_uint64\n#define tb_uint64    unsigned long long\n#endif\n\n";

        strSrc = "#include <string.h>\n#include <sstream>\n\nusing std::stringstream;\n\n#include \"" + iter->strTable + ".h\"\n\n";

        if (SUCCESS != AnalysisTable(*iter, strInc, strSrc)) {
            return;
        }

        strInc += "#endif\n";
        if (0 != (iRet = File::WriteStringToFile(strInc, iter->strTable + ".h"))) {
            ERRORLOG("Invalid argument.err msg=%s. %s\n", strerror(iRet), iter->strTable.c_str());
            return;
        }

        if (0 != (iRet = File::WriteStringToFile(strSrc, iter->strTable + ".cpp"))) {
            ERRORLOG("Invalid argument. err msg=%s. %s\n", strerror(iRet), iter->strTable.c_str());
            return;
        }
    }
}

int32 AutoTable::SplitStringByEnter(const string &strInput, vector<string> &vecContent) {
    if (strInput.empty()) {
        ERRORLOG("in put empty");
        return ERR_ARG;
    }
    vecContent.clear();

    int32 loopstus = 1;
    string strLine;
    uint32 uiStep;
    uint64 ullPos, ullHead = 0, ulltail = 0, ullWintail = 0, ullLinuxtail = 0;

    while (loopstus) {
        //查找两种换行符格式
        ullWintail = strInput.find("\r\n", ullHead);
        ullLinuxtail = strInput.find("\n", ullHead);
        if ((string::npos == ullWintail) && (string::npos == ullLinuxtail)) {
            loopstus = 0;
            ulltail = strInput.size();
        } else if (string::npos != ullWintail) {
            ulltail = ullWintail;
            uiStep = 2;
        } else {
            ulltail = ullLinuxtail;
            uiStep = 1;
        }

        strLine = strInput.substr(ullHead, ulltail - ullHead);
        ullHead = ulltail + uiStep;

        //过滤tab
        SplitStringByTab(strLine);
        if (strLine.empty()) {
            continue;
        }

        ullPos = strLine.find_first_not_of(' ');
        if (string::npos == ullPos) {
            continue;
        }

        strLine = strLine.substr(ullPos, strLine.size() - ullPos);
        vecContent.push_back(strLine);
    }

    return SUCCESS;
}

int32 AutoTable::GetTableFromVec(vector<string> &vecContent, vector<tagTable> &vecTable) {
    if (vecContent.empty()) {
        ERRORLOG("in put empty");
        return ERR_ARG;
    }

    int32 iStatus = 0;  //是否已经在分析sql语句，0:sql语句外；1：正在分析sql语句
    string strLine;
    uint64 ullPos, ullHead, ullTail;
    tagTable stTempTable;

    for (uint32 iIndex = 0; iIndex < vecContent.size(); iIndex++) {
        strLine = vecContent[iIndex];
        ullHead = ullPos = ullTail = 0;

        //过滤注释
        ullPos = strLine.find("/*");

        if (('/' == strLine[0]) && ('*' == strLine[1])) {
            //查找注释结尾
            while (string::npos == strLine.rfind("*/")) {
                iIndex++;

                if (iIndex < vecContent.size()) {
                    strLine = vecContent[iIndex];
                } else {
                    break;
                }
            }

            continue;
        }

        if ('#' == strLine[0]) {
            continue;
        }

        if ((0 == iStatus) && (string::npos != (ullHead = strLine.find("CREATE TABLE")))) {
            ullHead += 12;
            ullHead = strLine.find_first_not_of(' ', ullHead);
            if (string::npos == ullHead) {
                ERRORLOG("err line.%s", strLine.c_str());
                return ERR_UNKNOWN;
            }
            ullHead++;
            ullTail = strLine.find('`', ullHead);
            if (string::npos == ullTail) {
                ERRORLOG("err line.%s", strLine.c_str());
                return ERR_UNKNOWN;
            }

            stTempTable.strTable = strLine.substr(ullHead, ullTail - ullHead);
            stTempTable.vecRow.clear();
            iStatus = 1;
        } else if (1 == iStatus && ('`' == strLine[0])) {
            //列例子：`ikey` int(11) NOT NULL AUTO_INCREMENT,
            tagRow stTempRow;
            //找列名
            ullHead = strLine.find('`', 1);
            if (string::npos == ullHead) {
                ERRORLOG("err line.%s", strLine.c_str());
                return ERR_UNKNOWN;
            }

            stTempRow.strRowName = strLine.substr(1, ullHead - 1);

            //找列类型
            ullHead++;
            ullTail = strLine.find_first_not_of(' ', ullHead);
            if (string::npos == ullHead) {
                ERRORLOG("err line.%s", strLine.c_str());
                return ERR_UNKNOWN;
            }

            ullHead = ullTail;
            ullTail = strLine.find('(', ullHead);
            //判断ullTail与ullHead值差，防止查找到后面的相同字符
            if ((string::npos != ullHead) && (ullTail < ullHead + 10)) {
                stTempRow.strType = strLine.substr(ullHead, ullTail - ullHead);

                //找列长度
                ullTail++;
                ullHead = ullTail;
                ullTail = strLine.find(')', ullHead);
                if (string::npos == ullHead) {
                    ERRORLOG("err line.%s", strLine.c_str());
                    return ERR_UNKNOWN;
                }

                stTempRow.uiLength = atoi(strLine.substr(ullHead, ullTail - ullHead).c_str());
                //如果是整数型，查看是否是无符号整数
                ullHead = strLine.find("unsigned", ullHead);

                if (string::npos != ullHead) {
                    stTempRow.strType += " unsigned";
                }
            } else if (((string::npos != (ullTail = strLine.find("datetime", ullHead))) || (string::npos != (ullTail = strLine.find("DATETIME", ullHead)))) && (ullTail < ullHead + 10)) {
                stTempRow.strType = "datetime";
                stTempRow.uiLength = 19;
            } else {
                ERRORLOG("err line.%s", strLine.c_str());
                return ERR_UNKNOWN;
            }

            map<string, tagVarInfo>::iterator iter = m_mapVarInfo.find(stTempRow.strType);
            if (iter == m_mapVarInfo.end()) {
                ERRORLOG("err line.%s %s", strLine.c_str(), stTempRow.strType.c_str());
                return ERR_UNKNOWN;
            }

            stTempRow.stVarInfo = iter->second;
            stTempTable.vecRow.push_back(stTempRow);
        } else if (1 == iStatus && (')' == strLine[0])) {
            vecTable.push_back(stTempTable);
            iStatus = 0;
        } else {
            INFOLOG("line. %s", strLine.c_str());
        }
    }

    return SUCCESS;
}

void AutoTable::SplitStringByTab(string &strLine) {
    string strTemp;

    uint64 ullHead = 0, ulltail = 0;
    int32 loopstus = 1;
    while (loopstus) {
        ulltail = strLine.find("	", ullHead);
        if (string::npos == ulltail) {
            loopstus = 0;
            ulltail = strLine.size();
        }
        strTemp.append(strLine.substr(ullHead, ulltail - ullHead));
        strTemp.append(" ");
        ullHead = ulltail + 1;
    }

    strLine = strTemp;
}

int32 AutoTable::AnalysisTable(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    memset(szFun, 0, sizeof(szFun));
    snprintf(szFun, sizeof(szFun), "class %s {\n", stTable.strTable.c_str());
    strOutPutInc += szFun;

    strOutPutInc += "public:\n";
    CreateDefaultConstructor(stTable, strOutPutInc, strOutPutSrc);
    CreateCopyConstructor(stTable, strOutPutInc, strOutPutSrc);
    CreateDestruct(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateOperatorEqual(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateTableName(stTable, strOutPutInc, strOutPutSrc);
    CreateGetCount(stTable, strOutPutInc, strOutPutSrc);
    CreateSerialize(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateGetRowName(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateSetRowValue(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateGetRowValue(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateHas(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateClearHas(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\n";
    CreateIsValid(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "\nprivate:\n";
    CreateClassVariables(stTable, strOutPutInc, strOutPutSrc);

    strOutPutInc += "};\n\n";

    return SUCCESS;
}

void AutoTable::CreateDefaultConstructor(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    snprintf(szFun, sizeof(szFun), "    %s();\n", stTable.strTable.c_str());
    strOutPutInc += szFun;

    memset(szFun, 0, sizeof(szFun));
    snprintf(szFun, sizeof(szFun), "%s::%s() {\n", stTable.strTable.c_str(), stTable.strTable.c_str());
    strOutPutSrc += szFun;
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        if (0 != iter->stVarInfo.strVarType.compare(szString)) {
            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "    %s%s = 0;\n", iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
            strOutPutSrc += szFun;
        }
    }
    strOutPutSrc += "    m_ui_has_bit &= 0x00000000u;\n}\n\n";
}

void AutoTable::CreateCopyConstructor(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    snprintf(szFun, sizeof(szFun), "    %s(const %s& obj);\n", stTable.strTable.c_str(), stTable.strTable.c_str());
    strOutPutInc += szFun;

    memset(szFun, 0, sizeof(szFun));
    snprintf(szFun, sizeof(szFun), "%s::%s(const %s& obj) {\n", stTable.strTable.c_str(), stTable.strTable.c_str(), stTable.strTable.c_str());
    strOutPutSrc += szFun;
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        snprintf(szFun, sizeof(szFun), "    %s%s = obj.%s%s;\n", iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
        strOutPutSrc += szFun;
    }
    strOutPutSrc += "    m_ui_has_bit = obj.m_ui_has_bit;\n}\n\n";
}

void AutoTable::CreateDestruct(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    snprintf(szFun, sizeof(szFun), "    ~%s() {}\n", stTable.strTable.c_str());
    strOutPutInc += szFun;
}

void AutoTable::CreateGetRowName(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        snprintf(szFun, sizeof(szFun), "    inline const char* col_%s() const { return \"`%s`\"; }\n", iter->strRowName.c_str(), iter->strRowName.c_str());
        strOutPutInc += szFun;
    }
}

void AutoTable::CreateTableName(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    snprintf(szFun, sizeof(szFun), "    inline const char* table_name() const { return \"%s\"; }\n", stTable.strTable.c_str());
    strOutPutInc += szFun;
}

void AutoTable::CreateSetRowValue(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};
    uint32 uiIndex = 0;

    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        if (0 != iter->stVarInfo.strVarType.compare(szString)) {
            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "    bool set_%s(const %s value);\n", iter->strRowName.c_str(), iter->stVarInfo.strVarType.c_str());
            strOutPutInc += szFun;

            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "bool %s::set_%s(const %s value) {\n    %s%s = value;\n    m_ui_has_bit |= 0x%08x;\n    return true;\n}\n\n",
                     stTable.strTable.c_str(), iter->strRowName.c_str(), iter->stVarInfo.strVarType.c_str(),
                     iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), 1 << uiIndex);
            strOutPutSrc += szFun;
        } else {
            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "    bool set_%s(const %s& value);\n", iter->strRowName.c_str(), iter->stVarInfo.strVarType.c_str());
            strOutPutInc += szFun;

            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "bool %s::set_%s(const %s& value) {\n    if (value.size() > %u) {\n        m_ui_has_bit &= ~0x%08x;\n        return false;\n    }\n    %s%s = value;\n    m_ui_has_bit |= 0x%08x;\n    return true;\n}\n\n",
                     stTable.strTable.c_str(), iter->strRowName.c_str(), iter->stVarInfo.strVarType.c_str(), iter->uiLength,
                     1 << uiIndex, iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), 1 << uiIndex);
            strOutPutSrc += szFun;

            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "    bool set_%s(const char* value, size_t size);\n", iter->strRowName.c_str());
            strOutPutInc += szFun;

            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "bool %s::set_%s(const char* value, size_t size) {\n    if (size > %u) {\n        m_ui_has_bit &= ~0x%08x;\n        return false;\n    }\n    %s%s.assign(value, size);\n    m_ui_has_bit |= 0x%08x;\n    return true;\n}\n\n",
                     stTable.strTable.c_str(), iter->strRowName.c_str(), iter->uiLength, iter->uiLength,
                     iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), 1 << uiIndex);
            strOutPutSrc += szFun;

            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "    bool set_%s(const char* value);\n", iter->strRowName.c_str());
            strOutPutInc += szFun;

            memset(szFun, 0, sizeof(szFun));
            snprintf(szFun, sizeof(szFun), "bool %s::set_%s(const char* value) {\n    if (strlen(value) > %u) {\n        m_ui_has_bit &= ~0x%08x;\n        return false;\n    }\n    %s%s = value;\n    m_ui_has_bit |= 0x%08x;\n    return true;\n}\n\n",
                     stTable.strTable.c_str(), iter->strRowName.c_str(), iter->uiLength, iter->uiLength,
                     iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), 1 << uiIndex);
            strOutPutSrc += szFun;
        }
        uiIndex++;
    }
}

void AutoTable::CreateGetRowValue(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        if (0 != iter->stVarInfo.strVarType.compare(szString)) {
            snprintf(szFun, sizeof(szFun), "    inline %s %s() const { return %s%s; }\n", iter->stVarInfo.strVarType.c_str(), iter->strRowName.c_str(), iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
        } else {
            snprintf(szFun, sizeof(szFun), "    inline const %s& %s() const { return %s%s; }\n", iter->stVarInfo.strVarType.c_str(), iter->strRowName.c_str(), iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
        }

        strOutPutInc += szFun;
    }
}

void AutoTable::CreateHas(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};
    int32 uiIndex = 0;
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        snprintf(szFun, sizeof(szFun), "    inline bool has_%s() { return (m_ui_has_bit & 0x%08x) != 0; }\n", iter->strRowName.c_str(), 1 << uiIndex);
        strOutPutInc += szFun;
        uiIndex++;
    }
}

void AutoTable::CreateClearHas(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};
    int32 uiIndex = 0;
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        snprintf(szFun, sizeof(szFun), "    inline void clear_has_%s() { m_ui_has_bit &= ~0x%08x; }\n", iter->strRowName.c_str(), 1 << uiIndex);
        strOutPutInc += szFun;
        uiIndex++;
    }
}

void AutoTable::CreateOperatorEqual(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    snprintf(szFun, sizeof(szFun), "    %s& operator = (const %s& obj);\n", stTable.strTable.c_str(), stTable.strTable.c_str());
    strOutPutInc += szFun;

    memset(szFun, 0, sizeof(szFun));
    snprintf(szFun, sizeof(szFun), "%s& %s::operator =(const %s& obj) {\n    if (this == &obj) return *this;\n", stTable.strTable.c_str(), stTable.strTable.c_str(), stTable.strTable.c_str());
    strOutPutSrc += szFun;
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        snprintf(szFun, sizeof(szFun), "    %s%s = obj.%s%s;\n", iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
        strOutPutSrc += szFun;
    }
    strOutPutSrc += "    m_ui_has_bit = obj.m_ui_has_bit;\n    return *this;\n}\n\n";
}

void AutoTable::CreateGetCount(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    snprintf(szFun, sizeof(szFun), "    inline tb_uint32 get_count() const { return %ld; }\n", stTable.vecRow.size());
    strOutPutInc += szFun;
}

void AutoTable::CreateSerialize(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};

    strOutPutInc += "    string serialize() const;\n";

    memset(szFun, 0, sizeof(szFun));
    snprintf(szFun, sizeof(szFun), "string %s::serialize() const {\n    string strOutPut, strTemp;\n    stringstream strstr;\n\n", stTable.strTable.c_str());
    strOutPutSrc += szFun;
    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        if (0 != iter->stVarInfo.strVarType.compare(szString)) {
            snprintf(szFun, sizeof(szFun),
                     "    strstr.clear();\n    strstr.str("
                     ");\n    strstr << %s%s;\n    strstr >> strTemp;\n    strOutPut += \"%s=\" + strTemp + \"; \";\n\n",
                     iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str(), iter->strRowName.c_str());
        } else {
            snprintf(szFun, sizeof(szFun), "    strOutPut += \"%s=\" + %s%s + \"; \";\n\n", iter->strRowName.c_str(), iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
        }

        strOutPutSrc += szFun;
    }
    strOutPutSrc += "    return strOutPut;\n}\n\n";
}

void AutoTable::CreateClassVariables(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    char szFun[FUNCTION_LENGTH] = {0};
    strOutPutInc += "    tb_uint32 m_ui_has_bit;\n";

    for (vector<tagRow>::const_iterator iter = stTable.vecRow.begin(); iter != stTable.vecRow.end(); iter++) {
        memset(szFun, 0, sizeof(szFun));
        snprintf(szFun, sizeof(szFun), "    %s %s%s;\n", iter->stVarInfo.strVarType.c_str(), iter->stVarInfo.strVarPre.c_str(), iter->strRowName.c_str());
        strOutPutInc += szFun;
    }
}

void AutoTable::Init() {
    AddVarInfo("tinyint", "m_c_", "tb_int8");
    AddVarInfo("tinyint unsigned", "m_uc_", "tb_uint8");
    AddVarInfo("smallint", "m_s_", "tb_int16");
    AddVarInfo("smallint unsigned", "m_us_", "tb_uint16");
    AddVarInfo("int", "m_i_", "tb_int32");
    AddVarInfo("int unsigned", "m_ui_", "tb_uint32");
    AddVarInfo("bigint", "m_ll_", "tb_int64");
    AddVarInfo("bigint unsigned", "m_ull_", "tb_uint64");
    AddVarInfo("float", "m_f_", "float");
    AddVarInfo("double", "m_d_", "double");

    AddVarInfo("char", "m_str_", "string");
    AddVarInfo("varchar", "m_str_", "string");
    AddVarInfo("datetime", "m_str_", "string");
    AddVarInfo("timestamp", "m_str_", "string");

    AddVarInfo("TINYINT", "m_c_", "tb_int8");
    AddVarInfo("TINYINT unsigned", "m_uc_", "tb_uint8");
    AddVarInfo("SMALLINT", "m_s_", "tb_int16");
    AddVarInfo("SMALLINT UNSIGNED", "m_us_", "tb_uint16");
    AddVarInfo("INT", "m_i_", "tb_int32");
    AddVarInfo("INT UNSIGNED", "m_ui_", "tb_uint32");
    AddVarInfo("BIGINT", "m_ll_", "tb_int64");
    AddVarInfo("BIGINT UNSIGNED", "m_ull_", "tb_uint64");
    AddVarInfo("FLOAT", "m_f_", "float");
    AddVarInfo("DOUBLE", "m_d_", "double");

    AddVarInfo("CHAR", "m_str_", "string");
    AddVarInfo("VARCHAR", "m_str_", "string");
    AddVarInfo("DATETIME", "m_str_", "string");
    AddVarInfo("TIMESTAMP", "m_str_", "string");
}

void AutoTable::AddVarInfo(const char *szRowType, const char *szVarPre, const char *szVarType) {
    map<string, tagVarInfo>::iterator iter = m_mapVarInfo.find(szRowType);
    if (iter == m_mapVarInfo.end()) {
        tagVarInfo stTemp;
        stTemp.strVarPre = szVarPre;
        stTemp.strVarType = szVarType;
        m_mapVarInfo[szRowType] = stTemp;
    } else {
        ERRORLOG("repeat add.%s", szRowType);
    }
}

void AutoTable::CreateIsValid(const tagTable &stTable, string &strOutPutInc, string &strOutPutSrc) {
    int32 iTemp;
    int32 iMask = 0;

    for (size_t size = 0; size != stTable.vecRow.size(); ++size) {
        iTemp = iMask;
        iMask = 1 << size;
        iMask += iTemp;
    }
    char szFun[FUNCTION_LENGTH] = {0};
    snprintf(szFun, sizeof(szFun), "    inline bool is_valid() const {return (m_ui_has_bit & 0x%08x) != 0;}\n", iMask);

    strOutPutInc += szFun;
}

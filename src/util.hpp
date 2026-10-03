#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "cppjieba/Jieba.hpp"

namespace ns_util
{
    class FileUtil
    {
    public:
        static bool ReadFileToString(const std::string& file_path, std::string* content)
        {
            std::string line;

            std::ifstream ifs(file_path, std::ios::in);
            if(!ifs.is_open())
            {
                std::cerr << "Failed to open file: " << file_path << std::endl;
                return false;
            }

            while(getline(ifs, line))
            //如何理解getline读取到文件结束呢？？getline的返回值是一个&，while(bool), 本质是因为重载了强制类型转化
                *content += line;
            
            ifs.close();
            return true;
        }
    };

    class StringUtil
    {
    public:
        static bool SplitString(const std::string& str, const std::string& separator, std::vector<std::string>* result)
        {
            if(result == nullptr)
            {
                return false;
            }
            result->clear();

            boost::split(*result, str, boost::is_any_of(separator), boost::token_compress_on);
            return true;
        }
    };

    class JiebaUtil
    {
    private:
        static cppjieba::Jieba _jieba;
    public:
        static void JiebaSplit(const std::string& text, std::vector<std::string>* words)
        {
            _jieba.CutForSearch(text, *words);
        }
    };
    cppjieba::Jieba JiebaUtil::_jieba;
}
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "util.hpp"
#include <mutex>

namespace ns_index
{
    using DocInfo_t = struct DocInfo
    {
        std::string title;
        std::string content;
        std::string url;
        uint64_t doc_id;
    };

    using InvertedIndexElement_t = struct InvertedIndexElement
    {
        uint64_t doc_id;
        std::string word;
        int weight;
    };

    using InvertedList = std::vector<InvertedIndexElement_t>;

    typedef struct WordCnt
    {
        int title_count;
        int content_count;
    } WordCnt_t;

    class Index
    {
        static const int TITLE_WEIGHT = 10;
        static const int CONTENT_WEIGHT = 1;

    private:
        std::vector<DocInfo_t> _forward_index; //正排索引, doc_id -> 文档内容的映射
        std::unordered_map<std::string, InvertedList> _inverted_index; // 倒排索引, 关键词 -> 文档信息的映射
        static Index* _instance; // 单例实例
        static std::mutex _mutex; // 互斥锁，用于线程安全的单例初始化
    private:
        Index() {}
        Index(const Index&) = delete; // 禁止拷贝构造
        Index& operator=(const Index&) = delete; // 禁止拷贝赋值
        ~Index() {}

    public:
        static Index* GetInstance()
        {
            // 防止多线程大量请求锁资源
            if(nullptr == _instance)
            {
                // 建立互斥锁, 防止多线程进入临界区创建多分实例
                std::lock_guard<std::mutex> lock(_mutex);
                if(nullptr == _instance)
                {
                    _instance = new Index();
                }
            }
            return _instance;
        }

        DocInfo_t* GetDocInfoById(uint64_t doc_id)
        {
            if(doc_id >= _forward_index.size())
            {
                std::cerr << "Invalid doc_id: " << doc_id << std::endl;
                return nullptr;
            }
            return &(_forward_index[doc_id]);
        }

        InvertedList* GetInvertedIndexByWord(const std::string& word)
        {
            auto it = _inverted_index.find(word);
            if(it == _inverted_index.end())
            {
                return nullptr;
            }
            return &(it->second);
        }

        //建立索引
        bool BuildIndex(const std::string& input_file_path, const std::string& separator)
        {
            std::ifstream ifs(input_file_path, std::ios::in);
            if(!ifs.is_open())
            {
                std::cerr << "Failed to open input file: " << input_file_path << std::endl;
                return false;
            }

            int cnt = 0;
            std::string doc;
            while(std::getline(ifs, doc))
            {
                //1. 构建正排索引
                DocInfo_t* doc_info = BuildForwardIndex(doc, separator);
                if(nullptr == doc_info)
                {
                    std::cerr << "Failed to build forward index for line: " << doc << std::endl;
                    continue;
                }
                //2. 构建倒排索引
                if(!BuildInvertedIndex(*doc_info))
                {
                    std::cerr << "Failed to build inverted index for doc_id: " << doc_info->doc_id << std::endl;
                    continue;
                }
                cnt++;
                if(cnt % 100 == 0)
                {
                    std::cout << "成功构建 " << cnt << " 条索引" << std::endl;
                }
            }
            return true;
        }
    private: 
        //建立正排索引
        DocInfo_t* BuildForwardIndex(const std::string& doc, const std::string& separator)
        {
            std::vector<std::string> result;
            if(!ns_util::StringUtil::SplitString(doc, separator, &result))
            {
                std::cerr << "Failed to split line: " << doc << std::endl;
                return nullptr;
            }
            if(result.size() != 3)
            {
                std::cerr << "Invalid line format: " << doc << std::endl;
                return nullptr;
            }
            //走到这里, 说明文档成功被切分, 切被分为了三部分: title, content, url
            DocInfo_t doc_info;
            doc_info.title = result[0];
            doc_info.content = result[1];
            doc_info.url = result[2];
            doc_info.doc_id = _forward_index.size();
            _forward_index.push_back(std::move(doc_info));
            
            return &(_forward_index.back());
        }
        //建立倒排索引
        bool BuildInvertedIndex(const DocInfo_t& doc_info)
        {
            //1. 先分词
            std::vector<std::string> title_words, content_words;
            ns_util::JiebaUtil::JiebaSplit(doc_info.title, &title_words);
            ns_util::JiebaUtil::JiebaSplit(doc_info.content, &content_words);
            //2. 统计词频
            std::unordered_map<std::string, WordCnt_t> word_count;
            for(std::string word : title_words)
            {
                boost::to_lower(word);
                word_count[word].title_count++;
            }
            for(std::string word : content_words)
            {
                boost::to_lower(word);
                word_count[word].content_count++;
            }
            //3. 构建倒排索引
            for(const auto& pair : word_count)
            {
                std::string word = pair.first;
                WordCnt_t wc = pair.second;
                //根据词频计算权重
                InvertedIndexElement_t element;
                element.doc_id = doc_info.doc_id;
                element.word = word;
                element.weight = wc.title_count * TITLE_WEIGHT + wc.content_count * CONTENT_WEIGHT;
                //将元素加入倒排索引
                _inverted_index[word].push_back(std::move(element));
            }
            return true;
        }
    };
    Index* Index::_instance = nullptr;
    std::mutex Index::_mutex;
}

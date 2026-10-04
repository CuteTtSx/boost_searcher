#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "index.hpp"
#include "util.hpp"
#include <jsoncpp/json/json.h>

class Searcher
{
public:
    Searcher(const std::string &input_file_path, const std::string &separator)
        : _input_file_path(input_file_path), _separator(separator)
    {
    }
    ~Searcher()
    {
    }
    bool InitSearcher()
    {
        // 1. 获取索引对象
        _index = ns_index::Index::GetInstance();
        std::cout << "获取索引单例成功" << std::endl;

        // 2. 初始化索引
        if (!_index->BuildIndex(_input_file_path, _separator))
        {
            std::cerr << "Failed to build index from file: " << _input_file_path << std::endl;
            return false;
        }
        std::cout << "索引初始化完成" << std::endl;
        return true;
    }
    bool Search(const std::string &query, std::string *json)
    {
        // 1. 对query进行分词
        std::vector<std::string> query_words;
        ns_util::JiebaUtil::JiebaSplit(query, &query_words);
        // 2. 在索引中查找相关文档
        // 2.1 先根据query_words在倒排索引中查找倒排拉链的信息
        ns_index::InvertedList inverted_list_all;
        for (std::string word : query_words)
        {
            boost::to_lower(word);
            ns_index::InvertedList *inverted_list = _index->GetInvertedIndexByWord(word);
            // 如果该关键词无效, 则跳过该词
            if (!inverted_list)
                continue;
            // bug: 如果多个关键字的文档重复怎么办? 
            inverted_list_all.insert(inverted_list_all.end(), inverted_list->begin(), inverted_list->end());
        }
        // 3. 根据相关文档的权重进行排序
        std::sort(
            inverted_list_all.begin(),
            inverted_list_all.end(),
            [](const ns_index::InvertedIndexElement_t e1, const ns_index::InvertedIndexElement_t e2)
            {
                return e1.weight > e2.weight;
            });
        // 4. 构建json字符串输出
        Json::Value root;
        for(const auto& e : inverted_list_all)
        {
            //在正排索引中根据文档ID获取文档的详细信息
            ns_index::DocInfo_t* doc_info = _index->GetDocInfoById(e.doc_id);
            Json::Value doc;
            doc["title"] = doc_info->title;
            doc["desc"] = doc_info->content; // todo:摘要
            doc["url"] = doc_info->url;

            root.append(doc);
        }
        *json = Json::StyledWriter().write(root);

        return true;
    }

private:
    ns_index::Index *_index;
    std::string _input_file_path;
    std::string _separator;
};
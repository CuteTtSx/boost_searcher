#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "util.hpp"

namespace fs = boost::filesystem;

const std::string root_dir_name = "/home/zzy/boost_searcher/data/input/"; //一个目录, 存放所有html文件的根目录
const std::string output_file_name = "/home/zzy/boost_searcher/data/output/raw.txt"; //保存解析后的文档信息的文件路径
const std::string boost_url_head = "https://www.boost.org/doc/libs/1_78_0/doc/html/"; 
const char DOC_SEPARATOR = '\3'; //每个文档的分割符

typedef struct Docinfo
{
    std::string title;//文档标题
    std::string content;//文档内容
    std::string url;//文档url
} Docinfo_t;

//const &: 输入
//*: 输出
//&：输入输出
bool ParseFileName(const std::string& root_dir_name, std::vector<std::string>* file_paths);
bool ParseFileContent(const std::vector<std::string>& file_paths, std::vector<Docinfo_t>* docs);
bool SaveToFile(const std::vector<Docinfo_t>& docs, const std::string& output_file_name);

int main()
{
    std::vector<std::string> file_paths;
    // 1. 递归式的把每个html文件名带路径，保存到file_paths中，方便后期进行一个一个的文件进行读取
    if(!ParseFileName(root_dir_name, &file_paths))
    {
        std::cerr << "ParseFileName failed!" << std::endl;
        return 1;
    } 
    //debug
    // for(auto e: file_paths)
    //     std::cout << e << std::endl;
    
    std::vector<Docinfo_t> docs;
    // 2. 按照files_list读取每个文件的内容，并进行解析，生成每个文档的{title, content, url}信息
    if(!ParseFileContent(file_paths, &docs))
    {
        std::cerr << "ParseFileContent failed!" << std::endl;
        return 2;
    }

    // 3. 把解析完毕的各个文件内容，写入到output,按照\3作为每个文档的分割符
    if(!SaveToFile(docs, output_file_name))
    {
        std::cerr << "SaveToFile failed!" << std::endl;
        return 3;
    }
    
    return 0;
}

bool ParseFileName(const std::string& root_dir_name, std::vector<std::string>* file_paths)
{
    fs::path root_dir = fs::path(root_dir_name);

    // 路径不存在或者不是目录，就没必要走了
    if(!fs::exists(root_dir) || !fs::is_directory(root_dir))
    {
        std::cerr << root_dir_name <<  "does not exist or is not a directory!" << std::endl;
        return false;
    }

    fs::recursive_directory_iterator end; //定义一个空的迭代器，用来进行判断递归结束
    for(fs::recursive_directory_iterator iter(root_dir); iter != end; ++iter)
    {
        //判断文件是否是普通文件，html都是普通文件; 判断文件路径名的后缀是否符合要求
        if(!fs::is_regular_file(*iter) || iter->path().extension() != ".html")
            continue;
        
        // 走到这里的文件一定是一个合法的，以.html结束的普通网页文件
        file_paths->push_back(iter->path().string());//将所有带路径的html保存在files_list,方便后续进行文本分析
    }
    return true;
}

static bool ParseFileTitle(const std::string& file, std::string* title)
{
    // <title>xxxxxx</title>
    // 找到<title>和</title>之间的内容作为标题
    
    size_t start = file.find("<title>");
    if( start == std::string::npos)
        return false;

    size_t end = file.find("</title>");
    if(end == std::string::npos)
        return false;

    start += std::string("<title>").size();

    if(start > end)
        return false;

    *title = file.substr(start, end - start);
    return true;
}

static bool ParseFileContent(const std::string& file, std::string* content)
{
    //一个简易的状态机
    enum State { 
        LABEL, 
        CONTENT 
    }; 

    enum State state = LABEL;// 默认一开始都是LABEL
    for (char ch : file)
    {
        switch (state)
        {
            case LABEL:
                if(ch == '>')
                    state = CONTENT;
                break;
            case CONTENT:
                if(ch == '<')
                    state = LABEL;
                else
                {
                    //我们不想保留原始文件中的\n,因为我们想用\n作为html解析之后文本的分隔符
                    if(ch == '\n') 
                        ch = ' ';
                    *content += ch;
                }
                break;
            default:
                break;
        }
    }
    return true;
}

static bool ParseFileUrl(const std::string& file_path, std::string* url)
{
    fs::path path = fs::path(file_path);
    std::string url_head = boost_url_head;

    // std::string url_tail = path.filename().string(); // 注意: 不能这么写, 因为我们需要保留相对路径信息
    std::string url_tail = file_path.substr(root_dir_name.size());
    *url = url_head + url_tail;

    return true;
}

//DEBUG
static void ShowDocInfo(const Docinfo_t& doc)
{
    std::cout << "Title: " << doc.title << std::endl;
    std::cout << "Content: " << doc.content << std::endl;
    std::cout << "URL: " << doc.url << std::endl;
}

bool ParseFileContent(const std::vector<std::string>& file_paths, std::vector<Docinfo_t>* docs)
{
    for (const auto& file_path : file_paths)
    {
        std::string file;
        // 1. 取文件，Read();
        if(!ns_util::FileUtil::ReadFileToString(file_path, &file))
            continue;
        
        Docinfo_t doc;
        // 2. 解析指定的文件，提取title
        if(!ParseFileTitle(file, &doc.title))
            continue;
        //3. 解析指定的文件，提取content,就是去标签
        if(!ParseFileContent(file, &doc.content))
            continue;
         //4. 解析指定的文件路径，构建url
        if(!ParseFileUrl(file_path, &doc.url))
            continue;
        
        //debug
        // ShowDocInfo(doc);
        // break;

        //done, 此时一定是完成了解析任务，当前文档的相关结果都保存在了doc里面
        docs->push_back(std::move(doc)); // todo: 拷贝赋值开销大
    }
    return true;
}

//DEBUG
static void ReadDocInfo(const std::string &path, size_t n)
{
    std::ifstream ifs(path, std::ios::in);
    if(!ifs.is_open())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return;
    }
    for(int i = 0; i < n; ++i)
    {
        std::string line;
        if(std::getline(ifs, line))
        {
            std::cout << line << std::endl;
        }
    }

    ifs.close();
}

bool SaveToFile(const std::vector<Docinfo_t>& docs, const std::string& output_file_name)
{
    //按照二进制方式进行写入
    std::ofstream ofs(output_file_name, std::ios::out|std::ios::binary);
    if(!ofs.is_open())
    {
        std::cerr << "Failed to open output file: " << output_file_name << std::endl;
        return false;
    }
    for (const auto& doc : docs)
    {
        std::string line;

        line += doc.title;
        line += DOC_SEPARATOR;

        line += doc.content;
        line += DOC_SEPARATOR;

        line += doc.url;
        line += '\n';

        ofs << line;
    }
    ofs.close();

    //debug
    // ReadDocInfo(output_file_name, 2);

    return true;
}
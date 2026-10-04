#include "searcher.hpp"
#include <iostream>

int main()
{
    Searcher searcher("/home/zzy/boost_searcher/data/output/raw.txt", "\3");
    if (!searcher.InitSearcher())
    {
        std::cerr << "Failed to initialize searcher" << std::endl;
        return 1;
    }
    std::string query, results;
    while(std::getline(std::cin, query))
    {
        if (!searcher.Search(query, &results))
        {
            std::cerr << "Search failed for query: " << query << std::endl;
            continue;
        }
        std::cout << results << std::endl;
    }

    return 0;
}
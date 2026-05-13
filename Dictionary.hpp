#include"Radix.hpp"
#include"BKtree.hpp"
#include<filesystem>

enum class DICT{
    IRRUS,
    RUSIR
};

class Dictionary
{
private:
    OptimizedRadixTree m_radix;
    BKtree m_bkTree;

    std::vector<std::string> parse(const std::string& fileName,DICT d = DICT::RUSIR){
        std::vector<std::string> words;
        std::vector<char> buffer;
        if (d == DICT::RUSIR)
        {
            /* code */
            {
                std::ifstream file(fileName,std::ios::binary);
                if (!file.is_open())
                {
                    std::cerr<<"error"<<std::endl;
                    return {};
                }
                file.seekg(0, std::ios::end);
                auto size = file.tellg();
                file.seekg(std::ios::beg);
    
                buffer.resize(size);
    
                file.read(buffer.data(), size);
            }
    
            auto start = buffer.begin();
            auto curr = start;
            auto end = buffer.end();
    
            uint32_t stopSignPos = 0;
            uint32_t lines = 0;
            char32_t prev_c;
            while (curr != end)
            {
                char32_t c = utf8::next(curr, end);
                if (c == U' ' && prev_c == c)
                {
                    size_t curPos = curr - buffer.begin();
                    size_t startPos = start - buffer.begin();
    
                    // str.push_back(std::string(buffer.begin()+startPos, buffer.begin()+(curPos-2)));
                    std::string line = std::string(buffer.begin() + startPos, buffer.begin() + (curPos - 2));
                    if (!line.empty())
                    {
                        words.push_back(line);
                    }
    
                    auto wordStartIt = buffer.begin() + startPos;
                    auto wordEndIt = buffer.begin() + (curPos - 2);
    
                    while (curr != end && c != U'\n')
                    {
                        c = utf8::next(curr, end);
                    }
                    if (c == U'\n')
                    {
    
                        lines++;
                        stopSignPos = 0;
                        // c = utf8::next(curr,end);
                        start = curr;
                    }
                }
    
                prev_c = c;
            }
        }
        
        return words;
    }

    bool readBkTree(const std::string& fileName){
        if (!std::filesystem::exists("bkTree.bin"))
        {
            std::string wordsFileName = "shuffledWords.txt";
            if (!std::filesystem::exists(wordsFileName))
            {
                
                std::ofstream file(wordsFileName);
                if (!file.is_open())
                {
                    return false;
                }

                
            }
        }
        
        
    }
    bool readRadixTree(const std::string& fileName){
        std::vector<std::string> words;
        std::string binName;
        if (!std::filesystem::exists("rus-ir.bin"))
        { 
            words = parse(fileName);
            for (auto &&word : words)
            {
                m_radix.insert(word);
            }
            m_radix.serialize("rus-ir.bin");
        }else{
            m_radix.deserialize("rus-ir.bin");
        }
    }
public:
    bool read(const std::string& fileName){
        
    }
    
};
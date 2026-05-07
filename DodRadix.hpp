#include<stdint.h>
#include<vector>
#include<string>
#include<algorithm>
#include"utf8.h"
#include"json.hpp"

using namespace nlohmann;

class DodRadix
{
public:
    //NodeData
    std::vector<uint32_t> labelOffset;
    std::vector<uint32_t> labelLength;
    std::vector<uint32_t> firstChildOffset;
    std::vector<uint32_t> childrenCount;
    std::vector<uint32_t> parentOffset;
    std::vector<uint8_t> isEndOfWord;
    //ChildEntryData
    std::vector<char32_t> keys;
    std::vector<uint32_t> nodeOffset;
    //buffer
    std::vector<char> strBuffer;

    
public:
// Вспомогательные функции
    uint32_t addString(const std::string &str)
    {
        uint32_t offset = strBuffer.size();
        strBuffer.insert(strBuffer.end(), str.begin(), str.end());
        return offset;
    }

    std::string getString(uint32_t offset, uint32_t length) const
    {
        return std::string(strBuffer.data() + offset, length);
    }

    bool serializeToJson(const std::string& fileName){
        json j;
        j["labelOffset"] = labelOffset;
        j["labelLength"] = labelLength;
        j["firstChildOffset"] = firstChildOffset;
        j["childrenCount"] = childrenCount;
        j["parentOffset"] = parentOffset;
        j["isEnd"] = isEndOfWord;

        j["keys"] = keys;
        j["nodeOffset"] = nodeOffset;

        j["strBuffer"] = strBuffer;

        std::ofstream file(fileName);
        if(!file.is_open()){
            std::cerr<<"cant open file"<<std::endl;
            return false;
        }

        file<<j.dump();

        file.close();
        return true;
    }

    bool desirealizeFromJson(const std::string& fileName){
        std::ifstream file(fileName);
        if(!file.is_open()){
            std::cerr<<"cant open file"<<std::endl;
            return false;
        }
        json j;
        file>>j;

        labelOffset = j["labelOffset"].get<std::vector<uint32_t>>();
        labelLength = j["labelLength"].get<std::vector<uint32_t>>();
        firstChildOffset = j["firstChildOffset"].get<std::vector<uint32_t>>();
        childrenCount = j["childrenCount"].get<std::vector<uint32_t>>();
        parentOffset = j["parentOffset"].get<std::vector<uint32_t>>();
        isEndOfWord = j["isEnd"].get<std::vector<uint8_t>>();
        
        keys = j["keys"].get<std::vector<char32_t>>();
        nodeOffset = j["nodeOffset"].get<std::vector<uint32_t>>();
        
        
        strBuffer = j["strBuffer"].get<std::vector<char>>();
        //strBuffer.assign(str.begin(), str.end());

        return true;

    }

    // Поиск ребенка по первому символу
    uint32_t findChild(uint32_t nodeIdx, char32_t key) const
    {
        //const Node &node = nodes[nodeIdx];
        uint32_t childrenC = childrenCount[nodeIdx];
        uint32_t childOffset = firstChildOffset[nodeIdx];
        if (childrenC == 0)
            return UINT32_MAX;

        // Бинарный поиск, так как дети отсортированы
        auto start = keys.begin() + childOffset;
        auto end = start + childrenC;

        auto it = std::lower_bound(start, end, key,
                                   [](char32_t a, char32_t b)
                                   {
                                       return a < b;
                                   });

        

        if (it != end && *it == key)
        {
            uint32_t pos = it-keys.begin();
            if (pos<nodeOffset.size())
            {
                return nodeOffset[pos];
            }
            return UINT32_MAX;
        }
        return UINT32_MAX;
    }
    uint32_t findPrefixNode(uint32_t current, std::string& prefix) {
        //size_t pos = 0;
        auto startPrefix = prefix.begin();
        auto endPrefix = prefix.end();
        auto currPreifx = startPrefix;

        uint32_t prefixLen = prefix.length();
        uint32_t remindedPrefix = 0;
        size_t pos = 0;
        while (pos < prefix.length()) {
            //нужно сделать обаработку utf8 символов, наверное буду обрезать строку
            //и брать след символ

            std::string restPrefix = prefix.substr(pos);
            
            auto restPrefStart = restPrefix.begin();
            auto restPrefEnd = restPrefix.end();
            auto restPrefCurr = restPrefStart;

            char32_t firstChar32 = utf8::peek_next(restPrefCurr, restPrefEnd);


            //remindedPrefix = 
            
            uint32_t child_idx = findChild(current,firstChar32);
            
            //RadixNode* child = node->find_child(c);

            
            if(child_idx==UINT32_MAX){
                return UINT32_MAX;
            }
            //Node& child_node = nodes[child_idx];
            uint32_t offset = labelOffset[child_idx];
            uint32_t length = labelLength[child_idx];
            
            size_t restPrefLen = restPrefix.length();
            size_t chPrefLength = labelLength[child_idx];
            std::string child_pref = getString(offset,length);

           // std::wcout<<"node has "<<child->label<<" :"<<child->children.size()<<std::endl;


            size_t minLen = std::min(restPrefLen,chPrefLength);
            size_t commonLen = 0;


            auto childPrefStart = child_pref.begin();
            auto childPrefEnd = child_pref.end();
            auto childPrefCurr = childPrefStart;

            
            while (commonLen < minLen)
            {
                uint32_t cp1 = utf8::next(restPrefCurr, restPrefEnd);
                uint32_t cp2 = utf8::next(childPrefCurr, childPrefEnd);
                if (cp1 != cp2)
                {

                    break;
                }
                commonLen = restPrefCurr-restPrefStart;

            }
            //isCommonChild  

            if (commonLen == 0)
            {
                return UINT32_MAX;
            }else if(commonLen!= minLen){
                return UINT32_MAX;
            }else if(commonLen == restPrefLen){
                if (commonLen < chPrefLength)
                {
                    prefix += child_pref.substr(commonLen);
                }
                return child_idx;
            }

            pos += chPrefLength;
            current = child_idx;
        }
        //заглушка
        return current;
    }

    std::vector<std::string> findWordsWithPrefix(const std::string& prefix) {
        if(prefix == " "||prefix.empty()) return {};
        std::string mut_prefix = prefix;
        uint32_t node_idx = findPrefixNode(0, mut_prefix);

        if(node_idx==UINT32_MAX){
            return {};
        }

        auto found = getString(labelOffset[node_idx],labelLength[node_idx]);


       auto collect = 
        [&](auto&& self,uint32_t idx,const std::string& current,std::vector<std::string>& result)->void{
            
            if(isEndOfWord[idx]){
                result.push_back(current);
            }
            for(int i = 0;i<childrenCount[idx];i++){
                uint32_t childOffset = firstChildOffset[idx]+i;
                // ChildEntryUTF8& entry = children[childOffset];
                // Node& childNode = nodes[entry.nodeOffset];
                std::string child_prefix = getString(labelOffset[nodeOffset[childOffset]],labelLength[nodeOffset[childOffset]]);

                self(self,nodeOffset[childOffset],current+child_prefix,result);
            }
        };
        std::vector<std::string> result;
        collect(collect,node_idx,mut_prefix,result);


        return result;
    }


    
};
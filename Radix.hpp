#include <vector>
#include <iostream>
#include <algorithm>
#include <array>
#include <limits>
#include "utf8.h"
#include <windows.h>
#include <string>
#include <functional>
#include <fstream>
#include<unordered_set>

std::wstring string_to_wstring(const std::string &str, UINT codePage = CP_ACP)
{
    int size_needed = MultiByteToWideChar(codePage, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(codePage, 0, str.c_str(), -1, &wstr[0], size_needed);
    wstr.pop_back();
    return wstr;
}

// Компактное представление узла
struct Node
{
    uint32_t labelOffset;      // смещение в строковом буфере
    uint32_t labelLength;      // длина метки
    uint32_t firstChildOffset; // смещение в буфере детей
    uint32_t childrenCount;    // количество детей
    uint32_t parentOffset;     // смещение родителя (для обратной навигации)
    uint8_t isEndOfWord;       // флаг конца слова

    Node() : labelOffset(0), labelLength(0), firstChildOffset(0),
             childrenCount(0), parentOffset(0), isEndOfWord(0) {}
};

// Структура для связи "символ -> узел"
struct ChildEntry
{
    char key;            // первый символ метки ребенка
    uint32_t nodeOffset; // смещение узла ребенка

    ChildEntry(char k = 0, uint32_t off = 0) : key(k), nodeOffset(off) {}

    // Для сортировки
    bool operator<(const ChildEntry &other) const
    {
        return key < other.key;
    }
};
// Структура для связи "символ -> узел"
struct ChildEntryUTF8
{
    char32_t key;        // первый символ метки ребенка
    uint32_t nodeOffset; // смещение узла ребенка

    ChildEntryUTF8(char32_t k = 0, uint32_t off = 0) : key(k), nodeOffset(off) {}

    // Для сортировки
    bool operator<(const ChildEntryUTF8 &other) const
    {
        return key < other.key;
    }
};

class OptimizedRadixTree
{
private:
public:
    std::vector<char> prefixBuffer; // общий буфер для всех строк
    std::vector<char> translatBuffer;
    std::vector<Node> nodes;              // все узлы
    std::vector<ChildEntryUTF8> children; // все связи родитель-ребенок
    uint32_t childrenOffset;
    std::ofstream file2;

    // Вспомогательные функции
    uint32_t addString(const std::string &str)
    {
        uint32_t offset = prefixBuffer.size();
        prefixBuffer.insert(prefixBuffer.end(), str.begin(), str.end());
        return offset;
    }

    std::string getString(uint32_t offset, uint32_t length) const
    {
        return std::string(prefixBuffer.data() + offset, length);
    }

    // Поиск ребенка по первому символу
    uint32_t findChild(uint32_t nodeIdx, char32_t key) const
    {
        const Node &node = nodes[nodeIdx];
        if (node.childrenCount == 0)
            return UINT32_MAX;

        // Бинарный поиск, так как дети отсортированы
        auto start = children.begin() + node.firstChildOffset;
        auto end = start + node.childrenCount;

        auto it = std::lower_bound(start, end, ChildEntryUTF8(key, 0),
                                   [](const ChildEntryUTF8 &a, const ChildEntryUTF8 &b)
                                   {
                                       return a.key < b.key;
                                   });

        if (it != end && it->key == key)
        {
            return it->nodeOffset;
        }
        return UINT32_MAX;
    }

    // Добавление ребенка
    void addChild(uint32_t parentIdx, char32_t key, uint32_t childIdx)
    {
        
        Node &parent = nodes[parentIdx];
        Node &child = nodes[childIdx];
        //std::cout<<getString(child.labelOffset,child.labelLength)<<std::endl;
       // std::cout<<child.firstChildOffset<<std::endl;



        if(parent.childrenCount == 0){
            parent.firstChildOffset = children.size();
        }


        // Находим место для вставки (сохраняем сортировку)
        auto start = children.begin() + parent.firstChildOffset;
        auto end = start + parent.childrenCount;

        /* std::sort(start,end,[](ChildEntryUTF8& a,ChildEntryUTF8& b){
            return a.key<b.key;
        }); */

        auto pos = std::lower_bound(start, end, ChildEntryUTF8(key, 0),
                                    [](const ChildEntryUTF8 &a, const ChildEntryUTF8 &b)
                                    {
                                        return a.key < b.key;
                                    });

        // Вычисляем индекс в векторе children
        size_t insertPos = (pos - children.begin());


        

        // Вставляем нового ребенка
        children.insert(children.begin() + insertPos, ChildEntryUTF8(key, childIdx));

        

        // Обновляем firstChildOffset для всех узлов, у которых дети идут после вставки
        parent.childrenCount++;
        for(int i=0; i<nodes.size();i++){
            if (nodes[i].firstChildOffset > insertPos&& i!=childIdx)
            {
                nodes[i].firstChildOffset++;

                
            }
            
        }
        
        //print();
        if(children.size()>insertPos+1){
            uint32_t curParentOffset = children[insertPos].nodeOffset;
            uint32_t nextParentOffset = children[insertPos+1].nodeOffset;

            //в этом коде я проверяю следующего ребенка который после вставки был сдвинут
            // на 1 позицию 
            //если у нового ребенка и у сдвинутого разные родители.
            // мы обновляем offset у родителя сдвинутого ребенка, что бы он корректно указывал
            //на новое смещение своего первого ребенка

            Node& nextChildNode = nodes[nextParentOffset];
            Node& nextParentNode = nodes[nextChildNode.parentOffset];

            Node& currChildNode = nodes[curParentOffset];
            Node& currParentNode = nodes[currChildNode.parentOffset];
            
            if(nextParentNode.firstChildOffset==insertPos&&parent.childrenCount!=0&&currChildNode.parentOffset!=nextChildNode.parentOffset){
                std::string str = getString(nextChildNode.labelOffset,nextChildNode.labelLength);
                //std::cout<<"updated"<<std::endl;
                nextParentNode.firstChildOffset++;
            }
        }

        
        
        

        
    }

    // Обновление ребенка
    void updateChild(uint32_t parentIdx, char key, uint32_t newChildIdx)
    {
        Node &parent = nodes[parentIdx];

        auto start = children.begin() + parent.firstChildOffset;
        auto end = start + parent.childrenCount;

        auto it = std::lower_bound(start, end, ChildEntryUTF8(key, 0),
                                   [](const ChildEntryUTF8 &a, const ChildEntryUTF8 &b)
                                   {
                                       return a.key < b.key;
                                   });

        if (it != end && it->key == key)
        {
            it->nodeOffset = newChildIdx;
        }
    }

    // Удаление ребенка
    void removeChild(uint32_t parentIdx, char32_t key)
    {
        Node &parent = nodes[parentIdx];

        auto start = children.begin() + parent.firstChildOffset;
        auto end = start + parent.childrenCount;

        auto it = std::lower_bound(start, end, ChildEntryUTF8(key, 0),
                                   [](const ChildEntryUTF8 &a, const ChildEntryUTF8 &b)
                                   {
                                       return a.key < b.key;
                                   });

        if (it != end && it->key == key)
        {
            size_t removePos = (it - children.begin());
            children.erase(children.begin() + removePos);

            // Обновляем firstChildOffset
            for (Node &node : nodes)
            {
                if (node.firstChildOffset > removePos)
                {
                    node.firstChildOffset--;
                }
            }

            parent.childrenCount--;
        }
    }

public:
    OptimizedRadixTree()
    {
        // Создаем корневой узел
        Node root;
        root.labelOffset = addString("");
        root.labelLength = 0;
        root.firstChildOffset = 0;
        root.childrenCount = 0;
        root.parentOffset = UINT32_MAX;
        root.isEndOfWord = false;
        //children.resize(1);

        nodes.push_back(root);
        file2.open("log2.txt",std::ios::trunc);
        
        //children.resize(children.size()+1);
    }

    // *** ОСНОВНАЯ ФУНКЦИЯ INSERT **═*
    void insert(const std::string &word)
    {
        if (word.empty())
            return;

        uint32_t currentIdx = 0; // начинаем с корня
        // size_t wordPos = 0;        // текущая позиция в слове
        size_t wordLen = word.length();

        auto startWord = word.begin();
        auto endWord = word.end();
        auto currWord = startWord;

        while ((currWord-startWord)<wordLen)
        {
            // char firstChar = word[wordPos];

            char32_t firstChar32 = utf8::peek_next(currWord, endWord);
            // utf8::unchecked::utf32to8(firstChar32);

            // Ищем ребенка с таким первым символом
            uint32_t childIdx = findChild(currentIdx, firstChar32);

            if (childIdx == UINT32_MAX)
            {
                // Нет такого ребенка - создаем новый узел
                uint32_t newIdx = createNode(word.substr(currWord - startWord), currentIdx);
                addChild(currentIdx, firstChar32, newIdx);
            

                nodes[newIdx].isEndOfWord = true;
                return;
            }

            // Есть ребенок - проверяем совпадение метки
            Node &child = nodes[childIdx];
            std::string label = getString(child.labelOffset, child.labelLength);

            // Ищем общий префикс
            // size_t labelPos = 0;
            size_t labelLen = label.length();

            auto startLabel = label.begin();
            auto endLabel = label.end();
            auto currLabel = startLabel;

            /* while (wordPos < wordLen && labelPos < labelLen &&
                   word[wordPos] == label[labelPos]) {
                wordPos++;
                labelPos++;
            } */

            while ((currWord - startWord) < wordLen && (currLabel - startLabel) < labelLen)
            {
                uint32_t wordCP = utf8::next(currWord, endWord);
                uint32_t labelCP = utf8::next(currLabel, endLabel);

                if (wordCP != labelCP)
                {
                    utf8::prior(currWord, startWord);
                    utf8::prior(currLabel, startLabel);
                    break;
                }
            }
            size_t labelPos = currLabel - startLabel;
            size_t wordPos = currWord - startWord;

            if (labelPos == labelLen)
            {
                // Полное совпадение с меткой

                if (wordPos == wordLen)
                {
                    // Слово закончилось - помечаем узел как конец слова
                    nodes[childIdx].isEndOfWord = true;
                    return;
                }
                // Идем дальше по дереву
                currentIdx = childIdx;
                continue;
            }

            /* if (labelPos == labelLen) {
                // Полное совпадение с меткой
                if (wordPos == wordLen) {
                    // Слово закончилось - помечаем узел как конец слова
                    nodes[childIdx].isEndOfWord = true;
                    return;
                }
                // Идем дальше по дереву
                currentIdx = childIdx;
                continue;
            } */

            // Частичное совпадение - нужно разделить узел
            // Создаем разделительный узел с общим префиксом

            std::string commonPrefix = label.substr(0, labelPos);
            std::string childRemainder = label.substr(labelPos);
            std::string newRemainder = word.substr(wordPos);

            // 1. Создаем разделительный узел
            // uint32_t splitIdx = createNode(commonPrefix, currentIdx);

            uint32_t oldRemainderIdx = createEmptyNode(childIdx);
            Node &oldRemainder = nodes[oldRemainderIdx];
            Node &oldChild = nodes[childIdx];

            oldRemainder.labelOffset = oldChild.labelOffset + commonPrefix.length();
            oldRemainder.labelLength = childRemainder.length();
            oldRemainder.isEndOfWord = oldChild.isEndOfWord;
            oldRemainder.firstChildOffset = oldChild.firstChildOffset;
            oldRemainder.childrenCount = oldChild.childrenCount;

            auto startChildren = children.begin()+oldChild.firstChildOffset;
            auto endChildren = startChildren+oldChild.childrenCount;
            auto currChild = startChildren;

            while(currChild!=endChildren){
                uint32_t childIndex = currChild->nodeOffset;
                if(childIndex!=UINT32_MAX){
                    Node& childNode = nodes[childIndex];
                    childNode.parentOffset = oldRemainderIdx;
                }
                currChild++;
            }

           

            oldChild.childrenCount = 0;
            oldChild.firstChildOffset = children.size();
            oldChild.labelLength = commonPrefix.length();
            oldChild.isEndOfWord = false;

            auto RemIt = childRemainder.begin();
            auto RemEnd = childRemainder.end();
            char32_t remC2 = utf8::peek_next(RemIt, RemEnd);
            addChild(childIdx, remC2, oldRemainderIdx);

            

            // 4. Создаем узел для нового слова (если нужно)
            if (newRemainder.empty())
            {
                // Новое слово заканчивается на разделителе
                nodes[childIdx].isEndOfWord = true;
            }
            else
            {
                // Создаем новый узел для остатка
                
                uint32_t newRemainderIdx = createNode(newRemainder, childIdx);
                //oldChild.firstChildOffset = children.size();
                nodes[newRemainderIdx].isEndOfWord = true;
                auto newRemIt = newRemainder.begin();
                auto newRemEnd = newRemainder.end();
                char32_t remC = utf8::peek_next(newRemIt, newRemEnd);
                addChild(childIdx, remC, newRemainderIdx);
            }


            
            

            // Добавляем остаток старого ребенка как ребенка разделителя
            /* auto RemIt = childRemainder.begin();
            auto RemEnd = childRemainder.end();
            char32_t remC2 = utf8::peek_next(RemIt, RemEnd);
            addChild(childIdx, remC2, oldRemainderIdx); */

            // Удаляем старый узел
            // В реальном приложении нужно аккуратно управлять памятью
            // Здесь мы просто помечаем узел как удаленный или игнорируем
            // (для простоты оставляем, но в production нужно собирать мусор)

            return;
        }
    }

    // Создание нового узла
    uint32_t createNode(const std::string &label, uint32_t parentIdx)
    {
        Node newNode;
        newNode.firstChildOffset = children.size();
       /*  if(parentIdx!= UINT32_MAX){
            Node& parentNode = nodes[parentIdx];
            if (parentNode.firstChildOffset == children.size())
            {
                children.resize(parentNode.firstChildOffset+1);
                newNode.firstChildOffset = children.size();
            }
            
        } */
        newNode.labelOffset = addString(label);
        newNode.labelLength = label.length();


        newNode.childrenCount = 0;
        newNode.parentOffset = parentIdx;
        newNode.isEndOfWord = false;

        uint32_t newIdx = nodes.size();
        nodes.push_back(newNode);
        return newIdx;
    }

    uint32_t createEmptyNode(uint32_t parentIdx)
    {

        Node node;
        node.parentOffset = parentIdx;

        uint32_t newIdx = nodes.size();
        nodes.push_back(node);
        return newIdx;
    }

    uint32_t createNode(uint32_t labelfOffset, uint32_t labelLength, uint32_t parentIdx)
    {
        Node newNode;
        newNode.labelOffset = labelfOffset;
        newNode.labelLength = labelLength;
        newNode.firstChildOffset = children.size(); // новые дети будут добавлены позже
        newNode.childrenCount = 0;
        newNode.parentOffset = parentIdx;
        newNode.isEndOfWord = false;

        uint32_t newIdx = nodes.size();
        nodes.push_back(newNode);
        return newIdx;
    }
    std::vector<std::string> findWordsWithPrefix(const std::string& prefix) {
        std::string mut_prefix = prefix;
        uint32_t node_idx = findPrefixNode(0, mut_prefix);

        if(node_idx==UINT32_MAX){
            return {};
        }

        auto found = getString(nodes[node_idx].labelOffset,nodes[node_idx].labelLength);


        std::function<void(uint32_t,const std::string&,std::vector<std::string>&)> collect = 
        [&](uint32_t idx,const std::string& current,std::vector<std::string>& result){
            Node& node = nodes[idx];
            if(node.isEndOfWord){
                result.push_back(current);
            }
            for(int i = 0;i<node.childrenCount;i++){
                uint32_t childOffset = node.firstChildOffset+i;
                ChildEntryUTF8& entry = children[childOffset];
                Node& childNode = nodes[entry.nodeOffset];
                std::string child_prefix = getString(childNode.labelOffset,childNode.labelLength);

                collect(entry.nodeOffset,current+child_prefix,result);
            }
        };
        std::vector<std::string> result;
        collect(node_idx,mut_prefix,result);

        /* void collectWords(RadixNode * node, const std::wstring &current,
                          std::vector<std::wstring> &result)
        {
            if (node->is_end)
            {
                result.push_back(current);
            }
            for (auto child : node->children)
            {
                collectWords(child, current + child->label, result);
            }
        } */


        /* if (node) {
            
            
            collectWords(node, prefix, result);

        } */
        return result;
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
            Node& child_node = nodes[child_idx];
            
            size_t restPrefLen = restPrefix.length();
            size_t chPrefLength = child_node.labelLength;
            std::string child_pref = getString(child_node.labelOffset,child_node.labelLength);

           // std::wcout<<"node has "<<child->label<<" :"<<child->children.size()<<std::endl;


            size_t minLen = std::min(restPrefix.length(),chPrefLength);
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
                    prefix += getString(child_node.labelOffset,child_node.labelLength).substr(commonLen);
                }
                return child_idx;
            }

            pos += chPrefLength;
            current = child_idx;
        }
        //заглушка
        return current;
    }

    // Поиск слова
    bool search(const std::string &word) const
    {
        if (word.empty())
            return false;

        uint32_t currentIdx = 0;
        size_t wordPos = 0;
        size_t wordLen = word.length();
        auto wordStart = word.begin();
        auto wordEnd = word.end();
        auto currWord = wordStart;

        while ((currWord - wordStart) < wordLen)
        {
            // char firstChar = word[wordPos];
            char32_t firstChar32 = utf8::peek_next(currWord, wordEnd);
            //uint32_t cp = utf8::peek_next(currWord, wordEnd);

            uint32_t childIdx = findChild(currentIdx, firstChar32);

            if (childIdx == UINT32_MAX)
                return false;

            const Node &child = nodes[childIdx];
            std::string label = getString(child.labelOffset, child.labelLength);

            auto labelStart = label.begin();
            auto labelEnd = label.end();
            auto labelCurr = labelStart;

            while ((labelCurr - labelStart) < label.length())
            {
                if ((currWord - wordStart) < wordLen)
                {
                    uint32_t cp1 = utf8::next(labelCurr, labelEnd);
                    uint32_t cp2 = utf8::next(currWord, wordEnd);
                    if (cp1 != cp2)
                    {

                        return false;
                    }
                }
                else if((labelCurr - labelStart) == label.length()&&(currWord - wordStart) == wordLen){
                    return true;
                }else 
                {
                    return false;
                }
                
            }

            /* for (size_t i = 0; i < label.length(); i++) {
                if (wordPos >= wordLen || word[wordPos] != label[i]) {
                    return false;
                }
                wordPos++;
            }
             */
            // utf8::prior(currWord,wordStart);
            currentIdx = childIdx;
        }

        return nodes[currentIdx].isEndOfWord;
    }
    // Вспомогательная функция для отладки
    void print() const
    {
        printNode(0, 0);
    }

private:
    

    void printNode(uint32_t nodeIdx, int depth) const
    {
        std::function<void(uint32_t, int, std::ofstream &)> traverse = [&](uint32_t nodeIdx, int depth, std::ofstream &file)
        {
            const Node &node = nodes[nodeIdx];
            std::string label = getString(node.labelOffset, node.labelLength);

            // std::wstring label = string_to_wstring(labelU8,CP_UTF8);

            for (int i = 0; i < depth; i++)
                file<<"  ";
            file<<"├─"<<(label.empty() ? "''" : label);
            file<<"  has index: "<<nodeIdx;
            file<<" children:  "<<node.firstChildOffset<<","<<node.firstChildOffset+node.childrenCount;
            //file<<"  children count = "<<node.childrenCount;
            file<<"  with parent index:  "<<node.parentOffset;
            if (node.isEndOfWord)
                file << " (end)";
            file << "\n";

            for (uint32_t i = 0; i < node.childrenCount; i++)
            {
                const ChildEntryUTF8 &child = children[node.firstChildOffset + i];
                file<<" offset = "<<node.firstChildOffset+i;
                traverse(child.nodeOffset, depth + 1, file);
            }

        };
        std::ofstream file("log.txt",std::ios::app);
        file<<"\n";
        file<<"\n";
        traverse(nodeIdx,depth,file);

    }
    public:
    bool serialize(const std::string& fileName){
       std::ofstream file(fileName, std::ios::binary);

       if(!file.is_open()){
            return false;
       }else{
           uint32_t nodesSize = nodes.size();
           file.write(reinterpret_cast<char *>(&nodesSize), sizeof(uint32_t));
           file.write(reinterpret_cast<char *>(nodes.data()), nodesSize * sizeof(Node));

           uint32_t childrenSize = children.size();

           file.write(reinterpret_cast<char *>(&childrenSize), sizeof(uint32_t));
           file.write(reinterpret_cast<char *>(children.data()), childrenSize * sizeof(ChildEntryUTF8));

           uint32_t bufferSize = prefixBuffer.size();

           file.write(reinterpret_cast<char *>(&bufferSize), sizeof(uint32_t));
           file.write(prefixBuffer.data(), bufferSize);

           return true;
       }
    }

    bool deserialize(const std::string& fileName){
        std::ifstream file(fileName, std::ios::binary);

       if(!file.is_open()){
            return false;
       }else{
            nodes.clear();
            children.clear();
            prefixBuffer.clear();
           uint32_t nodesSize;
        
           file.read(reinterpret_cast<char*>(&nodesSize), sizeof(uint32_t));
           nodes.resize(nodesSize);

           file.read(reinterpret_cast<char*>(nodes.data()), nodesSize * sizeof(Node));

           uint32_t childrenSize;

           file.read(reinterpret_cast<char *>(&childrenSize), sizeof(uint32_t));
           children.resize(childrenSize);
           file.read(reinterpret_cast<char *>(children.data()), childrenSize * sizeof(ChildEntryUTF8));

           uint32_t bufferSize;

           file.read(reinterpret_cast<char *>(&bufferSize), sizeof(uint32_t));
           prefixBuffer.resize(bufferSize);
           file.read(prefixBuffer.data(), bufferSize);

           return true;
       }
    }


};

#include<vector>
#include<iostream>
#include<string>
#include<algorithm>

class BKtree
{
public:
struct Node
{
    uint32_t wordOffset;
    uint32_t wordLength;
    uint32_t firstChildOffset; // смещение в буфере детей
    uint32_t childrenCount;
    uint32_t parentOffset;
};
struct ChildEntry{
    uint32_t dist;
    uint32_t nodeOffset;

    ChildEntry(uint32_t d = 0, uint32_t off = 0) : dist(d), nodeOffset(off) {}

     // Для сортировки
    bool operator<(const ChildEntry &other) const
    {
        return dist < other.dist;
    }
};

private:
std::vector<Node> nodes;
std::vector<ChildEntry> children;
std::vector<char> words;
    
public:
    BKtree(/* args */);
    ~BKtree();
        // Вспомогательные функции
    uint32_t addString(const std::string &str)
    {
        uint32_t offset = words.size();
        words.insert(words.end(), str.begin(), str.end());
        return offset;
    }

    std::string getString(uint32_t offset, uint32_t length) const
    {
        return std::string(words.data() + offset, length);
    }
    // Создание нового узла
    uint32_t createNode(const std::string &label, uint32_t parentIdx)
    {
        Node newNode;
        newNode.firstChildOffset = children.size();
        
        newNode.wordOffset = addString(label);
        newNode.wordLength = label.length();


        newNode.childrenCount = 0;
        newNode.parentOffset = parentIdx;
       

        uint32_t newIdx = nodes.size();
        nodes.push_back(newNode);
        return newIdx;
    }

    void add_child(uint32_t parentIdx, uint32_t dist, uint32_t childIdx){

    };
    uint32_t find_child(uint32_t nodeIdx, uint32_t dist){

        const Node &node = nodes[nodeIdx];
        if (node.childrenCount == 0)
            return UINT32_MAX;

        // Бинарный поиск, так как дети отсортированы
        auto start = children.begin() + node.firstChildOffset;
        auto end = start + node.childrenCount;

        auto it = std::lower_bound(start, end, ChildEntry(dist, 0),
                                   [](const ChildEntry &a, const ChildEntry &b)
                                   {
                                       return a.dist < b.dist;
                                   });

        if (it != end && it->dist == dist)
        {
            return it->nodeOffset;
        }
        
        //заглушка
        return UINT32_MAX;
    };
    void check_boundaries(uint32_t dist){
        
    }

    void add_word(const std::string& word){
        /* if(!root){
            root =std::make_unique<Node>(word);
            return;
        }
        std::function<void(Node*, const std::wstring&)> insert =
        [&](Node* node, const std::wstring& w){
            int d = distance(node->word,w);
            if(d == 0) return;

            auto it = node->children.find(d);
            if (it == node->children.end())
            {
                node->children[d] = std::make_unique<Node>(w);
            }else{
                insert(it->second.get(),w);
            }
            
        }; 

        insert(root.get(), word);*/

    }
};



#include<vector>
#include<iostream>
#include<string>
#include<algorithm>
#include<functional>
#include"utf8.h"

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
    //BKtree(/* args */);
    //~BKtree();
        // Вспомогательные функции
    uint32_t add_string(const std::string &str)
    {
        uint32_t offset = words.size();
        words.insert(words.end(), str.begin(), str.end());
        return offset;
    }

    std::string get_string(uint32_t offset, uint32_t length) const
    {
        return std::string(words.data() + offset, length);
    }
    // Создание нового узла
    uint32_t create_node(const std::string &word, uint32_t parentIdx)
    {
        Node newNode;
        newNode.firstChildOffset = children.size();
        
        newNode.wordOffset = add_string(word);
        newNode.wordLength = word.length();


        newNode.childrenCount = 0;
        newNode.parentOffset = parentIdx;
       

        uint32_t newIdx = nodes.size();
        nodes.push_back(newNode);
        return newIdx;
    }
    

    void add_child(uint32_t parentIdx, uint32_t dist, uint32_t childIdx){
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

        auto pos = std::lower_bound(start, end, ChildEntry(dist, 0),
                                    [](const ChildEntry &a, const ChildEntry &b)
                                    {
                                        return a.dist < b.dist;
                                    });

        // Вычисляем индекс в векторе children
        size_t insertPos = (pos - children.begin());


        

        // Вставляем нового ребенка
        children.insert(children.begin() + insertPos, ChildEntry(dist, childIdx));

        

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
            //индекс узла нового ребенка
            uint32_t curNodeIndex = children[insertPos].nodeOffset;
            //индекс узла смещенного ребенка
            uint32_t shiftedNodeIndex = children[insertPos+1].nodeOffset;

            //в этом коде я проверяю следующего ребенка который после вставки был сдвинут
            // на 1 позицию 
            //если у нового ребенка и у сдвинутого разные родители,
            // мы обновляем offset у родителя узла сдвинутого ребенка, что бы он корректно указывал
            //на новое смещение своего первого ребенка
            //так же первое смещение ребенка у родителя, должно совпадать c позицией вставки
            //достаточно просто проверить firstChildOffset у родителя сдвинутого ребенка

            //узел смещенного ребенка
            Node& shiftedChildNode = nodes[shiftedNodeIndex];
            //родитель узла смещенного ребенка
            Node& shiftedParentNode = nodes[shiftedChildNode.parentOffset];
            
            //узел нового ребенка
            Node& currChildNode = nodes[curNodeIndex];
            //родитель нового ребенка
            Node& currParentNode = nodes[currChildNode.parentOffset];
            
            if(shiftedParentNode.firstChildOffset==insertPos&&parent.childrenCount!=0&&
            currChildNode.parentOffset!=shiftedChildNode.parentOffset){
                //std::string str = getString(shiftedChildNode.labelOffset,shiftedChildNode.labelLength);
                //std::cout<<"updated"<<std::endl;
                shiftedParentNode.firstChildOffset++;
            }
        }

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

    std::string try_correct(const std::string& wrongWord){

        if(nodes.empty()){
            return "";
        }
        std::string bestWord = "";
        uint32_t bestDist =  UINT32_MAX;
        std::function<void(uint32_t)> search = [&](uint32_t node_idx ){
            if(node_idx == UINT32_MAX){
                return;
            }
            Node& currentNode = nodes[node_idx];
            std::string word = get_string(currentNode.wordOffset,currentNode.wordLength);
            uint32_t d = distance(word,wrongWord);

            if (d < bestDist)
            {
                bestDist = d;
                bestWord = word;
            }
            //if(bestDist == 0) return;
            //if(bestDist<=2) return;
            

            auto start = children.begin()+currentNode.firstChildOffset;
            auto end = start+ currentNode.childrenCount;

            auto curr = start;

            while (curr!=end)
            {
                ChildEntry& child = *curr;

                if (child.dist<=(d+bestDist)&&child.dist>=(d-bestDist))
                {
                    search(child.nodeOffset);
                    //if(bestDist==0) return;
                }
                
                curr++;
            }
            
        };
        search(0);

        return (bestDist <= 2) ? bestWord : "";

        /* if(!root) return L"";

        std::wstring bestWord = L"";

        int bestDist = INT_MAX;
        int depth = 0;



        std::function<void(Node*,int,int&)> search = [&](Node* node, int curDepth, int &maxDepth){

            if(!node) return;

            int d = distance(node->word, wrongWord);
           


            if(d < bestDist){
                bestDist = d;
                bestWord = node->word;

                if(curDepth>maxDepth){
                    maxDepth = curDepth;
                }
            }


            if(bestDist == 0) return;

            for (const auto& [dist, child] : node->children)
            {
                if (dist <= d+ 2 && dist>= d-2)
                {
                    search(child.get(),curDepth+1, maxDepth);
                    if(bestDist == 0) return;
                }
                
                
            }
            

        };

        search(root.get(), 1 , depth); */
        



    }

    


    void add_word(const std::string& word){
        if (nodes.empty())
        {
            create_node(word, UINT32_MAX);
            return;
        }
        
        
        std::function<void(uint32_t, const std::string&)> insert =
        [&](uint32_t node_idx, const std::string& word){
            Node& node = nodes[node_idx];
            std::string node_word = get_string(node.wordOffset,node.wordLength);
            int d = distance(node_word,word);
            if(d == 0) return;

            uint32_t child_idx = find_child(node_idx,d);

            if(child_idx == UINT32_MAX){
                uint32_t new_child_idx = create_node(word,node_idx);
                add_child(node_idx,d,new_child_idx);
                
            }else{
                insert(child_idx, word);
            }
            
        }; 

        insert(0, word);

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

    int distance(const std::string &a, const std::string &b)
    {
        std::u32string a32 = utf8::utf8to32(a);
        std::u32string b32 = utf8::utf8to32(b);
        if (a32 == b32)
            return 0;
        if (a32.empty())
            return b32.length();
        if (b32.empty())
            return a32.length();

            

        int n = a32.length(), m = b32.length();

        std::vector<int> dp(m + 1);

        for (int j = 0; j <= m; j++)
            dp[j] = j;

       // int prev_prev_diag = 0;

        for (int i = 1; i <= n; i++)
        {
            int prev_diag = dp[0];
            dp[0] = i;

            //prev_prev_diag = i - 1;

            for (int j = 1; j <= m; j++)
            {
                int old_dp_j = dp[j];
                int cost = (a32[i - 1] == b32[j - 1]) ? 0 : 1;

                dp[j] = std::min({dp[j] + 1,
                                  dp[j - 1] + 1,
                                  prev_diag + cost});

                /* if (i > 1 && j > 1 &&
                    a[i - 1] == b[j - 2] &&
                    a[i - 2] == b[j - 1])
                {
                    dp[j] = std::min(dp[j], prev_prev_diag + 1);
                } */

               // prev_prev_diag = prev_diag;
                prev_diag = old_dp_j;
            }
        }
        return dp[m];
    }


    bool serialize(const std::string& pathName){
        std::ofstream file(pathName,std::ios::binary);

        if (!file.is_open())
        {
            return false;
        }
        else
        {
            uint32_t nodesSize = nodes.size();
            file.write(reinterpret_cast<char *>(&nodesSize), sizeof(uint32_t));
            file.write(reinterpret_cast<char *>(nodes.data()), nodesSize * sizeof(Node));

            uint32_t childrenSize = children.size();

            file.write(reinterpret_cast<char *>(&childrenSize), sizeof(uint32_t));
            file.write(reinterpret_cast<char *>(children.data()), childrenSize * sizeof(ChildEntry));

            uint32_t bufferSize = words.size();

            file.write(reinterpret_cast<char *>(&bufferSize), sizeof(uint32_t));
            file.write(words.data(), bufferSize);

            return true;
        }

        
    };
    bool deserialize(const std::string &pathName)
        {
            std::ifstream file(pathName, std::ios::binary);

            if (!file.is_open())
            {
                return false;
            }
            else
            {
                nodes.clear();
                children.clear();
                words.clear();
                uint32_t nodesSize;

                file.read(reinterpret_cast<char *>(&nodesSize), sizeof(uint32_t));
                nodes.resize(nodesSize);

                file.read(reinterpret_cast<char *>(nodes.data()), nodesSize * sizeof(Node));

                uint32_t childrenSize;

                file.read(reinterpret_cast<char *>(&childrenSize), sizeof(uint32_t));
                children.resize(childrenSize);
                file.read(reinterpret_cast<char *>(children.data()), childrenSize * sizeof(ChildEntryUTF8));

                uint32_t bufferSize;

                file.read(reinterpret_cast<char *>(&bufferSize), sizeof(uint32_t));
                words.resize(bufferSize);
                file.read(words.data(), bufferSize);

                return true;
            }
        };

};
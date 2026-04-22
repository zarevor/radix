#include<iostream>
#include<vector>
#include<unordered_map>
#include<string>
#include<memory>
#include<math.h>
#include<functional>


class BKTree{
    private:
    struct Node{
        std::wstring word;
        std::unordered_map<int,std::unique_ptr<Node>> children;
        Node(const std::wstring& w):word(w){}
    };
    std::unique_ptr<Node> root;

    int distance(const std::wstring& a, const std::wstring& b){
        if(a == b) return 0;
        if(a.empty()) return b.length();
        if(b.empty()) return a.length();

        int n = a.length(), m = b.length();
       /*  std::vector<int> prev(m+1), curr(m+1);
        for(int j=0;j<=m;j++)prev[j] =j;

        for (int i = 1; i <=n; i++)
        {
            
            curr[0] = i;
            for (int j = 1 ; j <=m; j++)
            {
                int cost = (a[i-1]==b[j-1])?0:1;
                curr[j] = std::min({prev[j]+1, curr[j-1]+1, prev[j-1]+cost});
            }
            std::swap(curr, prev);
            
            
        } */

        //if(std::abs(n-m)>2) return 2+1;
        /* if(n<m){
            return distance(b,a);
        } */

        /* std::vector<int> prev1(m+1), curr(m+1),prev2(m+1);
        for (int i = 0; i <= m; i++)
        {
            prev1[i] = i;
        }

        for (int i = 1; i <= n; i++)
        {
            curr[0] = i;
            int mindist = INT_MAX;

            for(int j = 1; j<=m; j++){
                int cost = (a[i-1] == b[j-1])?0:1;

                curr[j] == std::min({prev1[j]+1, curr[j-1]+1,prev1[j-1]+cost});

                if (i>1&&j>1&&a[i-1]==b[j-2]&&a[i-2]==b[j-1])
                {
                    curr[j] = std::min(curr[j], prev2[j-2]+1);
                }
                
            }
            prev2 =prev1;
            prev1=curr;
        }
        

        
         return prev1[m]; */



        std::vector<int>dp(m+1);

        for (int j = 0; j <= m; j++)
            dp[j] = j;

        int prev_prev_diag = 0;

        for (int i = 1; i <= n; i++)
        {
            int prev_diag = dp[0];
            dp[0] = i;

            prev_prev_diag = i-1;

            for (int j = 1; j <= m; j++)
            {
                int old_dp_j = dp[j];
                int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;

                dp[j] = std::min({dp[j] + 1,
                                  dp[j - 1] + 1,
                                  prev_diag + cost
                });

                if(i>1&&j>1&&
                    a[i-1]==b[j-2]&&
                    a[i-2] == b[j-1]){
                        dp[j] = std::min(dp[j],prev_prev_diag+1);
                    }

                prev_prev_diag = prev_diag;
                prev_diag = old_dp_j;
            }
        }
        return dp[m];
       
        
    }

    public: 
    void add(const std::wstring& word){
        if(!root){
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

        insert(root.get(), word);

    }


    std::wstring try_correct(const std::wstring& wrongWord){
        if(!root) return L"";

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

        search(root.get(), 1 , depth);
        std::wcout<<L"depth of word  "<<bestWord<<L"  ="<<depth<<std::endl;


        return (bestDist <= 2) ? bestWord : L"";

    }

    void findMaxDepth(Node *node, int currentdepth, int &maxDepth)
    {
        if (!node)
            return;

        if (currentdepth > maxDepth)
        {
            maxDepth = currentdepth;
        }

        for (const auto &child : node->children)
        {
            findMaxDepth(child.second.get(), currentdepth + 1, maxDepth);
        }
    }
    void maxDepth(int &maxDepth)
    {
        findMaxDepth(root.get(), 1, maxDepth);
    }
};
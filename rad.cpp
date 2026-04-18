#include <iostream>
#include <vector>
#include <string>
#include<algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include<fstream>
#include <clocale>
#include <windows.h>
#include<unordered_map>
#include<chrono>
#include<iterator>

struct RadixNode{
    std::wstring label;
    std::wstring translation;
    bool is_end;
    std::vector<RadixNode*> children;
    std::unordered_map<wchar_t, size_t> index;


    RadixNode(const std::wstring& l = L"", bool e = false, const std::wstring& tran = L"")
        : label(l), is_end(e),translation(tran) {}

    ~RadixNode() {
        for (auto child : children) {
            //std::wcout<<L"deleted Node   "<<label<<std::endl;
            
            delete child;
        }
    }
    RadixNode* find_child(wchar_t c) {

        /* for (auto& child:children)
        {
            if (c== child->label[0])
            {
                return child;
            }
            
        } */
        
        auto it = index.find(c);
        if(it!=index.end()){
            size_t ind = it->second;
            RadixNode* child = children[ind];

            if (child && ind <= children.size() && !children[ind]->label.empty())
            {
                return child;
            }
        }
        
        return nullptr;
    }

    int get_child_idx(wchar_t c){
        auto it = index.find(c);
        if(it!=index.end()){
            return it->second;
        }
        return -1;
    }
    void add_child(wchar_t c, RadixNode* node){
        index[c] = children.size();
        children.push_back(node);
    }
    void replace_child(RadixNode* new_child,wchar_t old_index_key, wchar_t new_index_key){
        if (old_index_key!=new_index_key)
        {
            auto it = index.find(old_index_key);
            if (it != index.end())
            {
                
                size_t old_child_index = index[old_index_key];
                index.erase(old_index_key);
                index[new_index_key] = old_child_index;
                children[old_child_index] = new_child;
            }
        }else{
            size_t child_index = index[old_index_key];
            children[child_index] = new_child;
        }

    }

};


class RadixTree{
    public:
    RadixNode* root;

    RadixTree() {
        root = new RadixNode();
    }

    ~RadixTree() {
        delete root;
    }

    // Найти общий префикс двух строк
    std::wstring commonPrefix(const std::wstring& s1, const std::wstring& s2) {
        std::wstring prefix;
        size_t min_len = std::min(s1.length(), s2.length());
        for (size_t i = 0; i < min_len; ++i) {
            if (s1[i] == s2[i]) {
                prefix += s1[i];
            } else {
                break;
            }
        }
        return prefix;
    }

    bool insert(RadixNode* node, const std::wstring& key, const std::wstring& trans = {}){
         if(key.empty()){
            if(!node->is_end){
                node->is_end = true;
                return true;
            }
            return false;
         }
        // std::wcout<<trans<<std::endl;
         RadixNode* child = node->find_child(key[0]);
         if(!child){
            node->add_child(key[0], new RadixNode(key,true,trans));
            return true;
         }
         std::wstring cp = commonPrefix(child->label,key);

         if(cp == child->label){
            return insert(child,key.substr(cp.length()),trans);
         }else if(!cp.empty()){
            RadixNode* parent = new RadixNode(cp, false);
            wchar_t old_child_c = child->label[0];
            child->label = child->label.substr(cp.length());
            //parent->children.push_back(child);
            parent->add_child(child->label[0],child);


            if(cp.length() == key.length()){
                parent->is_end = true;
                parent->translation = trans;
            }else {
                RadixNode* parent_child = new RadixNode(key.substr(cp.length()), true, trans);
                //parent->children.push_back(parent_child);
                parent->add_child(parent_child->label[0],parent_child);

            }
            /* auto it = std::find(node->children.begin(), node->children.end(), child);
            if (it != node->children.end()) {
                *it = parent;
            } */


            node->replace_child(parent,old_child_c, cp[0]);

            return true;
            
         }

         node->add_child(key[0],new RadixNode(key,true, trans));
         //node->children.push_back(new RadixNode(key, true));


         return true;

    }

    // Вставка ключа
    void insert(const std::wstring& key, const std::wstring& trans ={}) {
        insert(root, key, trans);
    }

    /* bool search(const std::wstring& key) const {
        RadixNode* node = root;
        size_t pos = 0;
        while (pos < key.length()) {
            char c = key[pos];
            RadixNode* child = node->findChild(c);
            if (!child || key.substr(pos).find(child->label) != 0) {
                return false;
            }
            pos += child->label.length();
            node = child;
        }
        return node->is_end;
    } */


     RadixNode* findPrefixNode(RadixNode* node,  std::wstring& prefix) {
        size_t pos = 0;
        while (pos < prefix.length()) {
            wchar_t c = prefix[pos];
            RadixNode* child = node->find_child(c);

            // здесь нужно доработать есть нюансы. child бывает nullptr. если нету искомого 
            if(child==nullptr){
                return nullptr;
            }
            
            std::wstring restPrefix = prefix.substr(pos);
            
            size_t rpLen = restPrefix.length();
            size_t clLen = child->label.length();

           // std::wcout<<"node has "<<child->label<<" :"<<child->children.size()<<std::endl;


            size_t minLen = std::min(restPrefix.length(), child->label.length());
            size_t commonLen = 0;
            while (commonLen < minLen && restPrefix[commonLen] == child->label[commonLen])
            {
                commonLen++;
            }
            //isCommonChild  

            if (commonLen == 0)
            {
                return nullptr;
            }else if(commonLen!= minLen){
                return nullptr;
            }else if(commonLen == rpLen){
                if (commonLen < clLen)
                {
                    prefix += child->label.substr(commonLen);
                }
                return child;
            }

            pos += child->label.length();
            node = child;
        }
        return node;
    }

    void collectWords(RadixNode* node, const std::wstring& current,
                     std::vector<std::wstring>& result) {
        if (node->is_end) {
            result.push_back(current);
        }
        for (auto child : node->children) {
            collectWords(child, current + child->label, result);
        }
    }

    std::vector<std::wstring> findWordsWithPrefix(const std::wstring& prefix) {
        if(prefix.empty()){
            return {};
        }
        std::wstring mutable_pref = prefix;
        RadixNode* node = findPrefixNode(root, mutable_pref);
        std::vector<std::wstring> result;
        if (node) {
            
            collectWords(node, mutable_pref, result);
            
        }
        return result;
    }

    RadixNode* search(const std::wstring& prefix) const {
        size_t pos = 0;
        RadixNode* node = root;
        while (pos < prefix.length()) {
            wchar_t c = prefix[pos];
            RadixNode* child = node->find_child(c);

            // здесь нужно доработать есть нюансы. child бывает nullptr. если нету искомого 
            if(child==nullptr){
                return nullptr;
            }
            
            std::wstring restPrefix = prefix.substr(pos);
            
            size_t rpLen = restPrefix.length();
            size_t clLen = child->label.length();

            //std::wcout<<"node has "<<child->label<<" :"<<child->children.size()<<std::endl;


            size_t minLen = std::min(restPrefix.length(), child->label.length());
            size_t commonLen = 0;
            while (commonLen < minLen && restPrefix[commonLen] == child->label[commonLen])
            {
                commonLen++;
            }
            //isCommonChild
            

            if (commonLen == 0)
            {
                return nullptr;
            }else if(commonLen!= minLen){
                return nullptr;
            }else if(commonLen == rpLen){
                
                if(child->is_end){

                    return child;
                }
            }


            pos += child->label.length();
            node = child;
        }
        if(node->is_end){
            return node;
        }
        return nullptr;
    }



};

std::wstring add_escapes(std::wstring& str)
{
	std::wstring out;
    for(size_t i = 0;i<str.length();i++){
        wchar_t curr = str[i];

        bool isCond = curr==L'б'||curr==L'в'||curr == L'а'||curr == L'о';

        if(std::isdigit(curr)){
            out+= L"\n\t";
            out +=curr;
        }else if(isCond&&(i+1)<str.length()&&str[i+1] == L'>'){
            out+=L"\n\t\t";
            out+=curr;
            out+='.';
        }else if(curr =='I'){
            out+=L"\n\t";

            size_t pos = i;
            while (i<str.length()&&str[i]=='I')
            {
                out+=str[i];
                i++;
            }
            
        }else{
            out+=curr;
        }
    }

    /* out.reserve(str.size()+15);
	size_t c = 0;
	while(c<str.length()){
		
		if(std::iswdigit(str[c])){
			out+= L"\n\t";
		}
		if(str[c] == L'>'){
			out.pop_back();
			c--;
			out+=L"\n\t\t";
			while(str[c]!='>'&&c<str.length()){
				out+=str[c];
				c++;
			}
		}
		out+=str[c];
		//std::wcout<<out<<std::endl;
		c++;
	} */
	//std::wcout<<out<<std::endl;
	return out;
}



int main()
{
    RadixTree tree;

   /*  tree.insert(L"avanes");
    tree.insert(L"app");
    tree.insert(L"application");
    tree.insert(L"appachi");
    tree.insert(L"ap");
    tree.insert(L"apus");
    tree.insert(L"avanagard");
    tree.insert(L"apply");
    tree.insert(L"banana");
    tree.insert(L"bandito");
    tree.insert(L"applica");
    tree.insert(L"bandila");

    auto res = tree.findWordsWithPrefix(L"applicat");

    for (auto &&i : res)
    {
        std::wcout<<i<<std::endl;
    } */


//1251 ru
    SetConsoleOutputCP(1251);  // кодировка консоли Windows-1251
    SetConsoleCP(1251);    
    char* loc = setlocale(LC_ALL,"Russian");

    //wchar_t cc = 'æ';
    char32_t oset = U'æ';
    const char* os = "æ";

    std::cout<<std::hex<<(uint32_t)oset<<std::endl;

    std::string filename = "mdict.dic";
    std::wifstream file(filename);

    

    // Проверка открытия файла
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return 1;
    }
    // file.imbue(std::locale("Russian"));


    std::wstring line;
    std::vector<std::wstring> data;
    
    
    while (std::getline(file, line)) {
        if (!line.empty())
        {
            size_t pos1 = line.find(L"  ");
            //size_t pos2 = line.find(L"1");
            std::wstring res1;

            if (pos1!=std::string::npos)
            {
               res1 = line.substr(0, pos1);
                
               if (std::iswalpha((res1[0]))||res1[0]=='_')
               {
                   auto escaped_line = add_escapes(line);
                  // std::wcout<<escaped_line<<std::endl;
                   tree.insert(res1, escaped_line);
               }

            }
        }
        
    }

    file.close(); // Необязательно: закроется автоматически
    std::cout<<tree.root->children.size()<<std::endl;

   /*  for (auto &&i : tree.root->children)
    {
        //std::wcout<<i->label<<std::endl;
        
    } */
    

    // Ищем по префиксу "app"

    std::wstring str;
    

    std::wstring pp;
    //std::cout<<words.size()<<std::endl;
    std::cout << "\nWords with prefix 'app':" << std::endl;

    while (std::getline(std::wcin, pp) && pp != L"out") {

        auto start = std::chrono::high_resolution_clock::now();


        std::vector<std::wstring> words = tree.findWordsWithPrefix(pp);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end-start);


        std::wcout<<"found "<<std::to_wstring(words.size())<<" words"<<"  with prefix:  "<<
        "'"<<pp<<"'"<<std::endl;
        for (const auto& word : words) {

            auto transs = tree.search(word);
            std::wcout<<transs->translation<<std::endl;
            
            //std::wcout << "  " << word << std::endl;
        } 

        std::cout<<"founded and collected "<<words.size()<<" words for  "<<duration.count()<<"  mc"<<std::endl;
    }

    system("pause");
    

    return 0;
}
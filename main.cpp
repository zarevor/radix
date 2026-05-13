#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include "utf8.h"
#include <locale>
#include <array>
#include "Radix.hpp"
#include <filesystem>
#include<fstream>
#include<fstream>
#include"BKtree.hpp"
#include<random>
#include"json.hpp"
#include<chrono>
#include"DodRadix.hpp"

void copyToDod(DodRadix& dod,OptimizedRadixTree& optRad){
    auto& nodes = optRad.nodes;
    auto& prefBuffer = optRad.prefixBuffer;
    auto& children = optRad.children;


    for (size_t i = 0; i < nodes.size(); i++)
    {
        dod.labelLength.push_back(nodes[i].labelLength);
        dod.labelOffset.push_back(nodes[i].labelOffset);
        dod.firstChildOffset.push_back(nodes[i].firstChildOffset);
        dod.childrenCount.push_back(nodes[i].childrenCount);
        dod.parentOffset.push_back(nodes[i].parentOffset);
        dod.isEndOfWord.push_back(nodes[i].isEndOfWord);
        
    }
    for (size_t i = 0; i < children.size(); i++)
    {
        dod.keys.push_back(children[i].key);
        dod.nodeOffset.push_back(children[i].nodeOffset);
    }
    dod.strBuffer = prefBuffer;
    
    
}

int main()
{
    BKtree bkTree;
    OptimizedRadixTree tree;
    auto startClock = std::chrono::high_resolution_clock::now();


     
    std::string fileName = "rus-ir.bin";
     if (!std::filesystem::exists(fileName))
    {
        std::vector<char> buffer;
        {
            std::ifstream file("mdict.txt", std::ios::binary);
            if (!file.is_open())
            {
                return 1;
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
        std::vector<std::string> str;
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

                        tree.insert(line);
                        //str.push_back
                        //bkTree.add_word(line);

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
        
        tree.serialize(fileName);
    }else
    {
        tree.deserialize(fileName);
    }

    if(!std::filesystem::exists("bkTree.bin")){

        std::ifstream file("lines.txt");
        if (!file.is_open())
        {
            std::cerr << "error" << std::endl;
        }
        std::string l;
        std::vector<std::string> lines;

        while (std::getline(file, l))
        {
            auto pos = l.find("\r");
            if (pos!=std::string::npos)
            {
                l = l.substr(0,pos);
                /* code */
            }
            
            if (!l.empty() && l != " ")
            {

                lines.push_back(l);
            }
        }
        std::random_device rd;
        std::mt19937 gen(rd());

        std::shuffle(lines.begin(), lines.end(), gen);
        for (size_t i = 0; i < lines.size(); i++)
        {
            bkTree.add_word(lines[i]);
        }
        bkTree.serialize("bkTree.bin");
    }else{
        bkTree.deserialize("bkTree.bin");
    }

    auto endClock = std::chrono::high_resolution_clock::now();


    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endClock-startClock);
    
    std::cout<<"время чтения бинарных файлов составило  "<<duration.count()<<"  миллисекунд"<<std::endl;

    
    std::cout<<"word  "<<(tree.search("яблоко")?"  exists":"  doesnt exists")<<std::endl;


    DodRadix dod;

    //copyToDod(dod,tree);

    //dod.serializeToJson("radix.json");

    dod.desirealizeFromJson("radix.json");

    OptimizedRadixTree treeTest;
    treeTest.insert("яблоко","apple");
    treeTest.insert("яблочный пирог","apple pie");
    treeTest.insert("яблоки","apples");
    treeTest.insert("яблочный мир","apple world");
    treeTest.insert("банан","banana");
    treeTest.insert("еда","food");
    treeTest.insert("есть","eating");

    std::cout<<"word  "<<(treeTest.search("яблоко")?"  exists":"  doesnt exists")<<std::endl;
    std::cout<<treeTest.findTranslation("яблочный мир")<<std::endl;
    

    std::string line;

    //tree.findPrefixNode


     while(std::getline(std::cin,line)){
        if(line.empty()||line==" ")
        {
          std::cout<<"пустая строка"<<std::endl;  
          continue;
        } 

        if(line == "стоп"){
            break;
        }
        auto s = std::chrono::high_resolution_clock::now();
        auto words = dod.findWordsWithPrefix(line);
        if(words.empty()){
            std::string corrected_line = bkTree.try_correct(line);
            if (!corrected_line.empty())
            {
                words = dod.findWordsWithPrefix(corrected_line);
            }
            
        }
        auto e =std::chrono::high_resolution_clock::now();

        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(e-s);
        std::cout<<"слова с префиксом  "<<"'"<<line<<"'"<<"  были найдены за ";
        std::cout<<dur.count()<<"  млс"<<std::endl; 

        if(words.empty()){
            
            std::cout<<"нет слов с префиксом  "<<line<<std::endl;
            continue;
        } 
        
        
        for (int i = words.size() - 1; i >= 0; i--)
        {

            std::cout<<words[i]<<std::endl;
            
        }
        std::cout<<"слова с префиксом  "<<line<<std::endl;
        
    
    } 


    system("pause");

    return 0;
}
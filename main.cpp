#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include "utf8.h"
#include <windows.h>
#include <locale>
#include <array>
#include "Radix.hpp"
#include <filesystem>
#include<fstream>
#include<fstream>

int main()
{

    OptimizedRadixTree tree;

    /* tree.insert("вагон");
    tree.insert("вагончик");
    tree.insert("вагонный");
    
    tree.insert("железо");
    tree.insert("корабль");
    tree.insert("железобетонный");
    tree.insert("живой");
    tree.insert("волшебный");
    tree.insert("во");
    tree.insert("здоровый");
    tree.insert("за");
    tree.insert("здрав");
    tree.print(); */

     SetConsoleOutputCP(CP_UTF8);
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

    std::ofstream file("words.txt");
    if(!file.is_open()){
        std::cerr<<"error"<<std::endl;
    }

    
   // std::cout<<"word  "<<(tree.search("хвастливый")?"  exists":"  doesnt exists")<<std::endl;
    std::string pref = "ман";
    auto result = tree.findWordsWithPrefix(pref);
    
    for (auto line : result)
    {
        file<<line;
        file<<"\n";
    }
    //file.close();




    std::string line;

    //tree.findPrefixNode


    while(std::getline(std::cin,line)){
        auto words = tree.findWordsWithPrefix(line);

        for (auto &&i : words)
        {
            std::cout<<i<<std::endl;
        }
        

        /* if(line == "стоп"){
            break;
        }
        std::cout<<"word  "<<line<<(tree.search(line)?"  exists":"  doesnt exists")<<std::endl; */
    }
    /* std::ofstream file("log.txt",std::ios::trunc);
    file.close();
    tree.insert("шалаш");
    tree.print();
    tree.insert("шалость");
    tree.print();
    tree.insert("шакал");
    tree.print();
    tree.insert("шалить");
    tree.print();

    tree.insert("шальной");
    tree.print();
    //удивительно. но когда я меняю смещение в ручную. дерево строится правильно.
    //значит где ошибка в addChild. пока не могу понять
    


    tree.insert("шаловливый");
    tree.print();


    tree.insert("шалашный");
    tree.print(); 
    tree.insert("шалашные");
    tree.print(); 
    tree.insert("шабашка");
    tree.print(); 
    tree.insert("шерсть");
    tree.print(); 
    tree.insert("шерстяной");
    tree.print(); 
    tree.insert("вровень");
    tree.print(); 

    tree.insert("кусать");
    tree.print(); 
    tree.insert("мастерить");
    tree.print(); 
    tree.insert("матрас");
    tree.print();  */



    /*  for (size_t i = 0; i < tree.children.size(); i++)
    {
        std::cout<<tree.children[i].key<<std::endl;
    } */

    std::cout << tree.nodes.size() << std::endl;
    std::cout<<"nodes count is  "<<tree.nodes.size()<<std::endl;
    std::cout<<"children count is  "<<tree.children.size()<<std::endl;


    


    //tree.insert("шаловливый");
    /* tree.insert("здоровый");
    tree.insert("задира");

    tree.insert("враг");
    tree.insert("шлем");
    tree.insert("шваль");
    tree.insert("шлак");

    tree.insert("шалун");
    

    std::cout << tree.nodes.size() << std::endl;
    std::cout<<"nodes count is  "<<tree.nodes.size()<<std::endl;
    std::cout<<"children count is  "<<tree.children.size()<<std::endl;


    //

    std::cout << tree.search("эксперимснтальный") << std::endl;
    std::cout << tree.search("алчность") << std::endl;
    std::cout << tree.search("вагон") << std::endl;
    std::cout << tree.search("траляля") << std::endl;
    std::cout << tree.search("шалун") << std::endl;
    std::cout << tree.search("шалунн") << std::endl;
    std::cout << tree.search("град") << std::endl;
    std::cout << tree.search("_г.") << std::endl;
    std::cout << tree.search("шаловливый") << std::endl;


   
    

    /* std::string in;
    while(std::getline(std::cin,in)){
        std::cout<<"type word to find in tree"<<std::endl;
        if (tree.search(in))
        {
            std::cout<<"word  "<<in<<"  found"<<std::endl;

        }else{
            std::cout<<"word  "<<in<<" not found"<<std::endl;
        }

    } */

    // tree.print();

    /* std::ofstream fileOut("lines.txt");



    if(fileOut.is_open()){
        for (size_t i = 0; i < str.size(); i++)
        {
            fileOut<<str[i]<<std::endl;
        }
        std::cout<<"writed"<<std::endl;

        fileOut.close();
    } */

    system("pause");

    return 0;
}
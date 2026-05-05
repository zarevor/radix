#include<iostream>

class IDictionary
{
private:
    
public:
    virtual ~IDictionary();
    virtual void parse(const std::string& fileName) = 0;
};
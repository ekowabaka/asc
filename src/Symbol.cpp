/* 
 * File:   Symbol.cpp
 * Author: ekow
 * 
 * Created on 25 November 2013, 20:56
 */

#include "Symbol.h"

Symbol::Symbol()
{
}

Symbol::Symbol(const Symbol& orig)
{
}

Symbol::~Symbol()
{
}

void Symbol::setDataType(std::string type)
{
    this->dataType = type;
}

std::string Symbol::getDataType()
{
    return this->dataType;
}

std::string Symbol::getIdentifier()
{
    
}


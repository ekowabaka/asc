#include <Parser.h>
#include <iostream>
#include <cstdlib>

Parser::Parser()
{
    generator = new Generator();
}

Parser::~Parser()
{
    
}

void Parser::setSource(std::string source)
{
	lexer = new Lexer(source);
    getToken();
}

void Parser::getToken()
{
    lookahead = lexer->getNextToken();

    switch(lookahead)
    {
        case INTEGER:
            std::cout<<"Integer"<<std::endl;
            break;
        case PLUS:
            std::cout<<"Plus"<<std::endl;
            break;
        case DIM:
            std::cout<<"Dim"<<std::endl;
            break;
    }
}

void Parser::parse()
{
    parseDeclaration();
	parseExpression();
}

bool Parser::match(Token token)
{
    if(lookahead == token)
    {
        return true;
    }
    else
    {
        std::cerr<<"Unexpected input"<<std::endl;
        exit(1);
    }
}

void Parser::parseDeclaration()
{
    if(lookahead == DIM)
    {
        std::string identifier;
        std::string datatype;
    
        getToken();
        match(IDENTIFIER);
        identifier = lexer->getIdentifierValue();
        getToken();
        match(AS);
        getToken();
        match(IDENTIFIER);
        datatype = lexer->getIdentifierValue();
        
        std::cout<<identifier<<" as "<<datatype<<std::endl;
    }
}

ExpressionNode * Parser::parseExpression()
{
    return parseExpression(false);
}

ExpressionNode * Parser::parseExpression(bool dontEmit)
{
    ExpressionNode *expression = parseTerm();
    ExpressionNode *left, *right;

    if(expression == NULL) return NULL;
    
    do{
        switch(lookahead)
        {
            case PLUS:
                left = expression;
                expression = new ExpressionNode();
                expression->setData(NODE_ADD);
                getToken();
                right = parseTerm();
                expression->setRight(right);
                expression->setLeft(left);
                break;
                
            case MINUS:
                left = expression;
                expression = new ExpressionNode();
                expression->setData(NODE_SUBTRACT);
                getToken();
                right = parseTerm();
                expression->setRight(right);
                expression->setLeft(left);
                break;
        }
    } while(lookahead == PLUS || lookahead == MINUS);
    
    if(dontEmit)
    {
        return expression;
    }
    else
    {
        if(expression != NULL) 
        {
            std::cout<<generator->emitExpression(expression);
            delete expression;
        }
    }
}

ExpressionNode * Parser::parseTerm()
{
    ExpressionNode *term = parseFactor();
    ExpressionNode *left, *right;
    
    do{
        switch(lookahead)
        {
            case MULTIPLY:
                left = term;
                term = new ExpressionNode();
                term->setData(NODE_MULTIPLY);
                getToken();
                right = parseFactor();
                term->setRight(right);
                term->setLeft(left);
                break;
                
            case DIVIDE:
                left = term;
                term = new ExpressionNode();
                term->setData(NODE_DIVIDE);
                getToken();
                right = parseFactor();
                term->setRight(right);
                term->setLeft(left);                
                break;
        }
    } while(lookahead == MULTIPLY || lookahead == DIVIDE);  
    
    return term;
}

ExpressionNode * Parser::parseFactor()
{
    ExpressionNode * factor = NULL;
    switch(lookahead) 
    {
        case INTEGER:
            factor = new ExpressionNode();
            factor->setData(lexer->getIntegerValue());
            getToken();
            break;
        
        case BRACKET_OPEN:
            factor = parseExpression(true);
            match(BRACKET_CLOSE);
            getToken();
            break;
    }
    std::cout<<factor<<std::endl;
    return factor;
}


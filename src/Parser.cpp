#include <Parser.h>
#include <iostream>
#include <cstdlib>
#include "generators/js/JsGenerator.h"
#include "LexerException.h"

Parser::OperatorLevel Parser::operators[] = {
	{{PLUS, MINUS}, {NODE_ADD, NODE_SUBTRACT}, 2},
	{{MULTIPLY, DIVIDE}, {NODE_MULTIPLY, NODE_DIVIDE}, 2}
};
int Parser::numOperators;

Parser::Parser(Generator * generator, std::string source)
{
    this->generator = generator;
    Parser::numOperators = sizeof(operators) / sizeof(OperatorLevel);
    symbolTable = new SymbolTable();
    setSource(source);    
}

Parser::~Parser()
{
    delete symbolTable;
}

void Parser::out(std::string type, std::string message)
{
    std::cerr<<
        lexer->getSourceFile()<<":"<<lexer->getLine()<<":"<<lexer->getColumn()<<
        ": "<<type<<": "<<message<<std::endl;
}

void Parser::error(std::string message)
{
    out("error", message);
    exit(1);
}

Symbol * Parser::insertSymbol(std::string identifier, std::string type)
{
    Symbol * symbol = symbolTable->insert(identifier, type);
    switch(symbolTable->getStatus())
    {
        case ADDED:
            if(type == "integer")
            {
                symbol->setAsNumber();
            }
            return symbol;
        case EXISTS:
            error(identifier + " has already been declared");
    }
}

Symbol * Parser::lookupSymbol(std::string identifier)
{
    Symbol * symbol = symbolTable->lookup(identifier);
    if(symbol == NULL)
    {
        error("Unknown identifier `" + identifier + "`");
    }
    return symbol;
}

void Parser::setSource(std::string source)
{
    try{
        lexer = new Lexer(source);        
        symbolTable->addType("number", "primitive");        
    }
    catch(LexerException * e)
    {
        std::cerr<<"Could not open file "<<source<<std::endl;
        exit(1);
    }
    getToken();
}

void Parser::getToken()
{
    lookahead = lexer->getNextToken();
}

void Parser::parse()
{
    do
    {
        parseDeclaration();
        parseIdentifierStatements();
        parsePrint();
        parseIf();
        
        // Detect newlines or EOFs
        if(lookahead != NEW_LINE && lookahead != END)
        {
            error("Unexpected token '" + lexer->getTokenString() + "'");
        }
        
        // Get the next token
        getToken();
        
        // Detect EOFs
        if(lookahead == END) break;
        
    }while(true);
}

bool Parser::match(Token token)
{
    if (lookahead == token)
    {
        return true;
    } 
    else
    {
        error("Unexpected " + Lexer::describeToken(lookahead) + " '" + lexer->getTokenString() + "'. Expected " + Lexer::describeToken(token) + ".");
    }
}

void Parser::parseDeclaration()
{
    if (lookahead == DIM)
    {
        std::string identifier;
        std::string datatype;
        std::string value;
    
        do{
            getToken();
            match(IDENTIFIER);
            identifier = lexer->getIdentifierValue();
            getToken();

            switch(lookahead)
            {
                case AS:
                    getToken();
                    match(IDENTIFIER);
                    datatype = lexer->getIdentifierValue();
                    getToken();
                    break;
                    
                case PERCENT:
                    datatype = "number";
                    getToken();
                    break;
            }
            
            if(!symbolTable->vaildateType(datatype) && datatype != "")
            {
                error("Unknown data type `" + datatype + "`");
            }
            
            currentSymbol = insertSymbol(identifier, datatype);
            generator->emitDeclaration(identifier, datatype);
            parseAssignment();
            generator->emitEndOfStatement();                        
        }
        while(lookahead == COMMA);
    }
}

void Parser::parseAssignment()
{
    if(lookahead == EQUALS)
    {
        getToken();
        generator->emitAssignment();
        ExpressionNode * expression = parseExpression();
        
        if(isNumeric(expression->getDataType()) && isNumeric(currentSymbol->getDataType()))
        {
            generator->emitExpression(expression);
        }
        else if(expression->getDataType() == currentSymbol->getDataType())
        {
            generator->emitExpression(expression);
        }        
        else if(currentSymbol->getDataType() == "")
        {
            currentSymbol->setDataType(expression->getDataType());
            generator->emitExpression(expression);
        }
        else
        {
            error(
                "Cannot assign value of type " + 
                expression->getDataType() + 
                " to variable " + currentSymbol->getIdentifier() + 
                " of type " + currentSymbol->getDataType()
            );
        }
    }

}

void Parser::parseIdentifierStatements()
{
    std::string identifier;
    if(lookahead == IDENTIFIER)
    {
        identifier = lexer->getIdentifierValue();
        currentSymbol = lookupSymbol(identifier);
        getToken();
        switch(lookahead)
        {
            case EQUALS:
                generator->write(identifier);
                parseAssignment();
                generator->emitEndOfStatement();
            break;
        }
    }
}

void Parser::parsePrint()
{
	if(lookahead == PRINT)
	{
		ExpressionNode * expression;
		getToken();
		expression = parseExpression();
		generator->emitPrint();
		generator->emitExpression(expression);
		generator->emitEndOfStatement();
	}
}

void Parser::parseIf()
{
	if(lookahead == IF)
	{

	}
}

ExpressionNode * Parser::parseExpression()
{
	return parseBinaryOperators(0, this);
}

ExpressionNode * Parser::parseBinaryOperators(int precedence, Parser * instance)
{
	ExpressionNode *expression;
	if(precedence < numOperators - 1)
	{
		expression = parseBinaryOperators(precedence+1, instance);
	}
	else
	{
		expression = parseUnaryOperators(instance);
	}

    if (expression == NULL) return NULL;

    ExpressionNode *left, *right;
	bool continueLooping;

    do
    {
    	continueLooping = false;

    	for(int i = 0; i < operators[precedence].numOperators; i++)
    	{
    		if(instance->lookahead == operators[precedence].tokens[i])
    		{
                left = expression;
                expression = new ExpressionNode();
                expression->setNodeType(operators[precedence].nodes[i]);
                instance->getToken();

            	if(precedence < numOperators - 1)
            	{
            		right = parseBinaryOperators(precedence+1, instance);
            	}
            	else
            	{
            		right = parseUnaryOperators(instance);
            	}

                expression->setRight(right);
                expression->setLeft(left);

                expression->setDataType(
					instance->resolveNumericTypes(
						left->getDataType(),
						right->getDataType()
					)
				);
                break;
    		}
    	}

    	for(int i = 0; i < operators[precedence].numOperators; i++)
    	{
    		if(instance->lookahead == operators[precedence].tokens[i])
    		{
    			continueLooping = true;
    			break;
    		}
    	}
    } while (continueLooping);

    return expression;
}

ExpressionNode * Parser::parseUnaryOperators(Parser * instance)
{
    ExpressionNode * factor = NULL;
    switch (instance->lookahead)
    {
        case INTEGER:
            factor = new ExpressionNode();
            factor->setIntegerValue(instance->lexer->getIntegerValue());
            factor->setDataType("integer");
            instance->getToken();
            break;
            
        case SINGLE:
            factor = new ExpressionNode();
            factor->setFloatValue(instance->lexer->getSingleValue());
            factor->setDataType("single");
            instance->getToken();
            break;
            
        case IDENTIFIER:
            factor = new ExpressionNode();
            factor->setIdentifierValue(instance->lexer->getIdentifierValue());
            factor->setDataType(instance->lookupSymbol(instance->lexer->getIdentifierValue())->getDataType());
            instance->getToken();
            break;

        case BRACKET_OPEN:
            factor = instance->parseExpression();
            instance->match(BRACKET_CLOSE);
            instance->getToken();
            break;

        case STRING:
        	factor = new ExpressionNode();
        	factor->setStringValue(instance->lexer->getStringValue());
        	factor->setDataType("string");
        	instance->getToken();
        	break;
    }
    return factor;
}

bool Parser::isNumeric(std::string datatype)
{
    if(datatype == "integer" || datatype == "single" || datatype == "double"  || 
       datatype == "long"    || datatype == "number")
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string Parser::resolveNumericTypes(std::string left, std::string right)
{
    if(left == "integer")
    {
        if(right == "integer") return "integer";
        if(right == "single") return "single";
    }
    else if(left == "single")
    {
        if(right == "single" || right == "integer") return "single";
    }    
}

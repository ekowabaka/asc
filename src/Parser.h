/*
 * Parser.h
 *
 *  Created on: 25 Oct 2013
 *      Author: ekow
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include "Lexer.h"

class Parser
{
private:
	Lexer * lexer;
	void parseExpression();
        Token lookahead;
public:
	void setSource(std::string source);
	void parse();
};

#endif /* PARSER_H_ */

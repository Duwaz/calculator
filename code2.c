/*
start := plus_minus
plus_minus := mul_div '+' plus_minus | mul_div '-' plus_minus | mul_div
mul_div := ln '*' mul_div | ln '/' mul_div | ln
ln := pow | 'ln' ln
pow := atom ^ pow | atom
atom := '(' plus_minus ')' | number
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Symbols {
	NUMBER = 'N', 
	LN = 'L',
	PLUS = '+', 
	MINUS = '-',
	MUL = '*', 
	DIV = '/',
	POW = '^', 
	LP = '(',
	RP = ')',
	NUL = 0
};

typedef struct Token {
	enum Symbols type;
	double value;
	struct Token* next;
} Token;

typedef struct AST_t {
	int answer;
} AST_t;

AST_t* plus_minus(Token*);


void freeList(Token* token) {
	while (token != NULL) {
		Token* next = token->next;
		free(token);
		token = next;
	}
}

int Parser(char** str, Token** token) {
	*token = (Token*)malloc(sizeof(Token));
	while (**str == ' ' || **str == '\n') (*str)++;
	switch (**str) {
		case '\0':
			free(*token);
			*token = NULL;
			return 1; 

		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
		case '(':
		case ')':
			(*token)->type = (enum Symbols)(*(*str)++);
			break;

		default: {
			if (**str == '.' || (**str >= '0' && **str <= '9')) {
				(*token)->type = NUMBER;
				sscanf_s(*str, "%lf", &((*token)->value));
				do {
					++(*str);
				} while (**str == '.' || (**str >= '0' && **str <= '9'));
			}
			else if (!strncmp(*str, "ln", 2)) {
				(*token)->type = LN;
				*str += 3;
			}
			else {
				free(*token);
				*token = NULL;
				return 0;
			}
		}
	}
	return 1;
}
Token* Lexer(char** str) {
	Token* first;
	Token* current;
	Parser(str, &first);
	current = first;
	while (current) {
		Token* next;
		if (!Parser(str, &next)) {
			current->next = NULL;
			freeList(first);
			return NULL;
		}
		current->next = next;
		current = next;
	}
	return first;
}

AST_t* eval(char* str) {
	Token* tokens;

	tokens = Lexer(&str);
	if (tokens == NULL) return 0;
	return plus_minus(tokens);
}

int main() {
	char str[256];
	AST_t* result;
	float out;
	printf("Enter an expression: ");
	fgets(str, sizeof(str), stdin);
	result = eval(str);
	if (result) 
	{
		out=result->answer; 
		printf("Answer: %lf\n\n", out);
	} 
	else puts("Error in expression");
	getchar();
}

///////////////////////////////////////////////////////////////////////////////////////////////

AST_t* mul_div(Token* token)
{
	Token* dop;
	AST_t* right = (AST_t *) malloc (sizeof(AST_t));
	AST_t* left = (AST_t *) malloc (sizeof(AST_t));
	left->answer = token->value;

	if (token->next && (MUL == token->next->type || DIV == token->next->type) )
 	{
		if (token->next->next == NULL) return NULL;
		right = mul_div(token->next->next);

		if (MUL == token->next->type) token->value = left->answer*right->answer;
		if (DIV == token->next->type) token->value = left->answer/right->answer;
		
		dop = token->next->next->next;
		free(token->next->next);
		free(token->next);
		token->next = dop;

		left->answer=token->value;
 	}
	free(right);
	return left;
}

AST_t* plus_minus(Token* token)
{
	Token* dop;
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (token->next && (PLUS == token->next->type || MINUS == token->next->type) )
 	{
		if (! token->next->next) return NULL;
		right = plus_minus(token->next->next);
		if (! right) return NULL;

		if (PLUS == token->next->type) token->value = left->answer+right->answer;
		if (MINUS == token->next->type) token->value = left->answer-right->answer;
		free(right);

		dop = token->next->next->next;
		free(token->next->next);
		free(token->next);
		token->next = dop;

		left->answer = token->value;
 	}
	return left;
}

/*
Вот ваш код с доски

AST_t* plus_minus()
{
	AST_t* left = mul_div();
	if (! left) return NULL;
	if accept(Tok_PLUS)
 	{
		AST_t* right = plus_minus();
		if (! right) return NULL;
 	}
}

0) память будет выделяться на right и left в последней стадии(*осмотр на открывающуюся скобку и на число [atom])
1) определять переменные после начала кода не всегда удаётся(не знаю почему)
2) хранить наши распаршенные части в глобальной памяти не есть хорошо...

AST_t* plus_minus(Token* token)
{
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;
	if accept(Tok_PLUS, token)
 	{
		right = plus_minus(token);
		if (! right) return NULL;
 	}
}

3) AST_t по идее достаточно только поле "значение".
4) Тогда accept(Tok_PLUS, token) превращается в 'Tok_PLUS==token->next->type'
  *Главное, чтобы внутри процедуры мы сокращали начальную(введённую) строку.

AST_t* plus_minus(Token* token)
{
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;
	if (Tok_PLUS == token->next->type)
 	{
		right = plus_minus(token);
		if (! right) return NULL;		
 	}
}

5) Т.к. мы рассматриваем token->next->type, то нужно проверить, что token->next не нулевой

AST_t* plus_minus(Token* token)
{
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (! token->next) return left;
	if (Tok_PLUS == token->next->type)
 	{
		right = plus_minus(token);
		if (! right) return NULL;		
 	}
}

6) теперь строка 'right = plus_minus(token)' не верна, переписываем на 'right = plus_minus(token->next->next)'
7) нужно проверить нет ли ошибки при вводе так что проверяем если после "+" число.
8) для return можем использовать любую переменую либо left, либо right, тогда пусть воспользуемся left, а right удалим из памяти.

AST_t* plus_minus(Token* token)
{
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (! token->next) return left;
	if (Tok_PLUS == token->next->type)
 	{
		if (!сtoken->next->next) return NULL;
		right = plus_minus(token->next->next);
		if (! right) return NULL;
		
		left->answer = left->answer + right->answer;
		free(right);
		return left;
 	}
}

9) так, теперь обрежем нашу распаршенную "исходную" строку
9.1) можно здесь освободить память 

AST_t* plus_minus(Token* token)
{
	Token *dop;
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (! token->next) return left;
	if (Tok_PLUS == token->next->type)
 	{
		if (! token->next->next) return NULL;
		right = plus_minus(token->next->next);
		if (! right) return NULL;

		token->value = left->answer + right->answer;
		free(right);

		dop = token->next->next->next;
		free(token->next->next);
		free(token->next);
		token->next = dop;
		
		left->answer = token->value;
		return left;
 	}
}

10) если "if (Tok_PLUS == token->next->type)" будет false, значит просмотр этой части окончен

AST_t* plus_minus(Token* token)
{
	Token *dop;
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (! token->next) return left;
	if (Tok_PLUS == token->next->type)
 	{
		if (! token->next->next) return NULL;
		right = plus_minus(token->next->next);
		if (! right) return NULL;

		token->value = left->answer + right->answer;

		dop = token->next->next->next;
		free(token->next->next);
		free(token->next);
		token->next = dop;
		
		left->answer = token->value;
		free(right);
		return left;
 	}
	return left;
}

11) 'return left' можно совместить

AST_t* plus_minus(Token* token)
{
	Token* dop;
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (! token->next) return left;
	if (Tok_PLUS == token->next->type)
 	{
		if (! token->next->next) return NULL;
		right = plus_minus(token->next->next);
		if (! right) return NULL;

		token->value = left->answer + right->answer;
		free(right);

		dop = token->next->next->next;
		free(token->next->next);
		free(token->next);
		token->next = dop;

		left->answer = token->value;
 	}
	return left;
}

12) строку "if (! token->next) return left;" тоже можно совместить

AST_t* plus_minus(Token* token)
{
	Token* dop;
	AST_t* right;
	AST_t* left = mul_div(token);
	if (! left) return NULL;

	if (token->next && PLUS == token->next->type )
 	{
		if (! token->next->next) return NULL;
		right = plus_minus(token->next->next);
		if (! right) return NULL;

		token->value = left->answer+right->answer;
		free(right);

		dop = token->next->next->next;
		free(token->next->next);
		free(token->next);
		token->next = dop;

		left->answer = token->value;
 	}
	return left;
}
*/

/*
start := plus_minus
plus_minus := (number '+' mul_div | number '-' mul_div | mul_div) [пока не конец данной строки] plus_minus
mul_div := number '*' ln | number '/' ln | ln
ln := 'ln' pow | pow
pow := number ^ atom | atom
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
	RP = ')'
};

typedef struct Token {
	enum Symbols type;
	double value;
	struct Token* next;
} Token;

Token* plus_minus(Token*);

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

double eval(char* str, double* res) {
	Token* tokens;
	Token* result;
	tokens = Lexer(&str);
	if (tokens == NULL) return 0;
	result = plus_minus(tokens);
	if (!result || result->type != NUMBER ) { //|| result->next != NULL
		freeList(tokens);
		return 0;
	}
	*res = result->value;
	freeList(tokens);
	return 1;
}

int main() {
	char str[256];
	double result;
	printf("Enter an expression: ");
	fgets(str, sizeof(str), stdin);
	eval(str, &result) ? printf("Answer: %lf\n\n", result) : puts("Error in expression");
	getchar();
}

///////////////////////////////////////////////////////////////////////////////////////////////

Token* atom(Token* token)
{
	Token* dop_token;
	Token* result;
	
	dop_token = token->next;

	if(token->type == LP)
	{
		result = plus_minus(token->next);

		token->type = NUMBER;
		token->value = result->value;
		token->next = result->next;
	}
	else
	if(dop_token->type == RP)
	{
		token->next = dop_token->next;
	}

	return token;
}

Token* our_pow(Token* token)
{
	Token* dop_token;
	Token* result;

	dop_token = token->next;

	if(dop_token != NULL && dop_token->type == POW)
	{
		result = atom(dop_token->next);

		token->next = result->next;
		token->type = NUMBER;

		token->value = pow(token->value, result->value);
	}
	else
	{
		token = atom(token);
	}
	return token;
}

Token* ln(Token* token)
{
	Token* result;

	if(token->next != NULL && token->type == LN )
	{
		result = our_pow(token->next);

		token->next = result->next;
		token->type = NUMBER;

		token->value = log(result->value);
	}
	else
	{
		token = our_pow(token);
	}
	return token;
}

Token* mul_div(Token* token)
{
	Token* dop_token;
	Token* result;
	
	dop_token = token->next;

	if(dop_token != NULL && (dop_token->type == MUL || dop_token->type == DIV))
	{
		result = ln(dop_token->next);
			
		token->next = result->next;
		token->type = NUMBER;

		if (dop_token->type == MUL) 
		{
			token->value = token->value * result->value;
		}
		if (dop_token->type == DIV) 
		{
			token->value = token->value / result->value;
		}
	}
	else
	{
		token = ln(token);
	}
	return token;
}

Token* plus_minus(Token* token)
{
	Token* dop_token;
	Token* result;
	
	dop_token = token->next;

	if(dop_token != NULL && (dop_token->type == PLUS || dop_token->type == MINUS))
	{
		result = mul_div(dop_token->next);
		
		token->next = result->next;
		token->type = NUMBER;	

		if (dop_token->type == PLUS) 
		{
			token->value = token->value + result->value;
		}
		if (dop_token->type == MINUS) 
		{
			token->value = token->value - result->value;
		}	
	}
	else
	{
		token = mul_div(token);
	}

	if (token->next != NULL)
	{
		return plus_minus(token);
	}
	else
		return token;
}

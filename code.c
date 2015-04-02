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

Token* calculator(Token*, int);

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
		if (next == 0) { current->next = (Token*)malloc(sizeof(Token)); break; }
		current->next = next;
		current = next;
	}
	current->next->type = NUL;
	current->next->next = NULL;
	return first;
}

double eval(char* str, double* res) {
	Token* tokens;
	Token* result;

	tokens = Lexer(&str);
	if (tokens == NULL) return 0;
	result = calculator(tokens, 0);
	if (!result || result->type != NUMBER ) { //|| result->next != NULL
		freeList(tokens);
		return 0;
	}
	*res = result->value;
	free(result);
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

Token* calculator(Token* token, int control_signal)
{
	Token* dop_token; 
	Token* for_free;

	if (control_signal != 2 && control_signal < 5)
	{
		token = calculator(token, control_signal+1);
	}

	switch (control_signal)
	{
		case 0 : 
			if (token->next->type == PLUS || token->next->type == MINUS)
			{
				dop_token = calculator(token->next->next, 0);

				if (token->next->type == PLUS)
				{
					token->value += dop_token->value;
				}
				if (token->next->type == MINUS)
				{
					token->value -= dop_token->value;
				}

				token->type = NUMBER;
				for_free = token->next;
				token->next = dop_token->next;
				free(for_free->next);
				free(for_free);
				return token;
			}
			break;
		
		case 1 : 
			if (token->next->type == MUL || token->next->type == DIV)
			{
				dop_token = calculator(token->next->next, 1);

				if (token->next->type == MUL)
				{
					token->value *= dop_token->value;
				}
				if (token->next->type == DIV)
				{
					token->value /= dop_token->value;
				}

				token->type = NUMBER;
				for_free = token->next;
				token->next = dop_token->next;
				free(dop_token);
				free(for_free);
				return token;
			}
			break;

		case 2 : 
			if (token->type == LN)
			{
				dop_token = calculator(token->next, 2);					
				token->value = log(dop_token->value);
				token->type = NUMBER;
				for_free = token->next;
				token->next = dop_token->next;
				free(for_free);
				return token;
			}
			else
			{
				return calculator(token, 3);
			}
			break;

		case 3 : 
			if (token->next->type == POW)
			{
				dop_token = calculator(token->next->next, 3);					
				token->value = pow(token->value, dop_token->value);
				token->type = NUMBER;
				for_free = token->next;
				token->next = dop_token->next;
				free(for_free->next);
				free(for_free);
				return token;
			}
			break;

		case 4 :
			if (token->type == LP)
			{
				for_free = token;
				token = token->next;
				free(for_free);
				token = calculator(token, 0);
				for_free = token->next;
				token->next = for_free->next;
				free(for_free);
				return token;
			}
			if (token->type == NUMBER)
			{
				return token;
			}

	default:
		break;
	}
	return token;
}

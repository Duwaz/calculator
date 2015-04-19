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
	RP = ')'
};

typedef struct Token {
	enum Symbols type;
	double value;
	struct Token* next;
} Token;

typedef struct AST_t {
	int data;
	enum Symbols type;
	struct AST_t* left;
	struct AST_t* right;
} AST_t;

AST_t* plus_minus(Token*);
float calculate(AST_t*);

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
	return plus_minus(&tokens);
}

int main() {
	char str[256];
	AST_t* result;
	printf("Enter an expression: ");
	fgets(str, sizeof(str), stdin);
	result = eval(str);
	if (result) 
	{
		printf("Answer: %lf\n\n", calculate(result));
	} 
	else puts("Error in expression");
	getchar();
}

/*	//////////////////////////////////////////////////////////////////////////////////////	*/

int accept(enum Symbols type, Token** token)
{
	if (!(*token)) return NULL;
	if ((*token)->type != type) return NULL;
	(*token)=(*token)->next;
	return 1;
}

int expect(enum Symbols type, Token** token)
{
	if (!token) return NULL;
	if ((*token)->type != type) return NULL;
	return 1;
}

/*	//////////////////////////////////////////////////////////////////////////////////////	*/

AST_t* atom(Token** token)
{
	AST_t *output;
	if (accept(LP, token))
 	{
		output = plus_minus(token);
		if (accept(RP, token) ) return output;

		return NULL;
 	}
	else 
		if (expect(NUMBER, token))
		{
			output = (AST_t*) malloc (sizeof(AST_t));
			output->type=NUMBER;
			output->data=(*token)->value;
			output->left=NULL;
			output->right=NULL;
			(*token)=(*token)->next;

			return output;
		}
		else
			return NULL;
}

AST_t* sub(Token** token)
{
	AST_t* right, *output;
	AST_t* left = atom(token);
	if (! left) return NULL;
	if (accept(POW, token))
 	{
		right = sub(token);
		if (! right) return NULL;

		output = (AST_t*) malloc (sizeof(AST_t));
		output->type=POW;
		output->left = left;
		output->right = right;

		return output;
 	}
	else return left;
}

AST_t* ln(Token** token)
{
	AST_t* left, *output;
	if (accept(LN, token))
 	{
		left = ln(token);
		if (! left) return NULL;

		output = (AST_t*) malloc (sizeof(AST_t));
		output->type=LN;
		output->left = left;
		output->right = NULL;

		return output;
 	}
	else return sub(token);
}

AST_t* mul_div(Token** token)
{
	enum Symbols type;
	AST_t* right, *output;
	AST_t* left = ln(token);
	if (! left) return NULL;
	if (accept(type = MUL, token) || accept(type = DIV, token))
 	{
		right = mul_div(token);
		if (! right) return NULL;

		output = (AST_t*) malloc (sizeof(AST_t));
		output->type=type;
		output->left = left;
		output->right = right;

		return output;
 	}
	else return left;
}

AST_t* plus_minus(Token** token)
{
	enum Symbols type;
	AST_t* right, *output;
	AST_t* left = mul_div(token);
	if (! left) return NULL;
	if (accept(type = PLUS, token) || accept(type = MINUS, token))
 	{
		right = plus_minus(token);
		if (! right) return NULL;
		
		output = (AST_t*) malloc (sizeof(AST_t));
		output->type=type;
		output->left = left;
		output->right = right;

		return output;
 	}
	else return left;
}

/*	//////////////////////////////////////////////////////////////////////////////////////	*/

float calculate(AST_t* tree)
{
	float answer;

	if (tree->type == NUMBER) return tree->data;

	if (tree->type == PLUS) return calculate(tree->left)+calculate(tree->right);
	if (tree->type == MUL) return calculate(tree->left)*calculate(tree->right);
	if (tree->type == MINUS) return calculate(tree->left)-calculate(tree->right);
	if (tree->type == DIV) return calculate(tree->left)/calculate(tree->right);
	if (tree->type == LN) return log(calculate(tree->left));
	if (tree->type == POW) return pow(calculate(tree->left),calculate(tree->right));
}

// PL0.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
// pl0Lab.cpp : ???????应??????诘恪?
//


// pl0 compiler source code
#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

  //////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

  //////////////////////////////////////////////////////////////////////
  // gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '|') //2017-09-24
	{
		getch();
		if (ch == '|') { sym = SYM_OR; getch(); }
		else sym = SYM_NULL;
	}
	else if (ch == '&')//2017-09-24
	{
		getch();
		if (ch == '&') { sym = SYM_AND; getch(); }
		else sym = SYM_NULL;
	}
	else if (ch == '/')
	{
		getch();
		if (ch == '/') { cc = ll; getch(); getsym(); } // line comment
		else if (ch == '*') { // block-comment
			getch(); //skip '*'
			while (1) {
				if (ch == '*') {
					getch();
					if (ch == '/') break;
				}
				getch();
			}
			getch();
			getsym();
		}
		else sym = SYM_SLASH;
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

  //////////////////////////////////////////////////////////////////////
  // generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

  //////////////////////////////////////////////////////////////////////
  // tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

  //////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

		 // enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*)&table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*)&table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

  //////////////////////////////////////////////////////////////////////
  // locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

  //////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else	error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

  //////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

  //////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

  //////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	void or_expr(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

							   //while (inset(sym, facbegsys)) commented!
	if (inset(sym, facbegsys))//2017-10-27
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*)&table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			//2018-11-23
			getsym();

		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			or_expr(set);//expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);//2017-10-27
			gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NOT) // UMINUS,  Expr -> '!' Expr
		{
			getsym();
			or_expr(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		else if (sym == SYM_ODD) //2017-09-24
		{
			getsym();
			expression(fsys);
			gen(OPR, 0, 6);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if (inset(sym, facbegsys))
} // factor

  //////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

  //////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression


  //////////////////////////////////////////////////////////////////////
void rel_expr(symset fsys) //condition
{
	int relop;
	symset set;

	/* //2017-0924
	if (sym == SYM_ODD)
	{
	getsym();
	expression(fsys);
	gen(OPR, 0, 6);
	}
	else
	*/
	{
		set = uniteset(relset, fsys);
		expression(set);
		//destroyset(set);
		if (!inset(sym, relset))
		{
			//error(20);//2017-09-24
		}
		else //if (inset(sym, relset))
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
		destroyset(set);
	} // else
} // rel_expr , condition


void and_expr(symset fsys)
{
	symset set;

	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));

	rel_expr(set);
	while (sym == SYM_AND)
	{
		getsym();
		rel_expr(set);
		gen(OPR, 0, OPR_AND);

	} // while

	destroyset(set);
}
//////////////////////////////////////////////////////////////////////
void or_expr(symset fsys)
{
	symset set;

	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));

	and_expr(set);
	while (sym == SYM_OR)
	{
		getsym();
		and_expr(set);
		gen(OPR, 0, OPR_OR);

	} // while

	destroyset(set);
} // or_expr
//2018-11-29
void assign(symset fsys) {
	symset set;

	set = uniteset(fsys, createset(SYM_BECOMES));

	expression(set);
	if (code[cx - 1].f == LOD) {//说明是一个单独的变量id_var
		if (sym == SYM_BECOMES) {//说明是赋值
			int id_var_level = code[cx - 1].l;//expression递归到factor
								//生成了STO指令，其中有层差和偏移地址信息，保留下来
			int id_var_address = code[cx - 1].a;
			cx--;//因为后面还有‘:=’，抛弃STO指令
			getsym();
			assign(fsys);
			gen(STO, id_var_level, id_var_address);//传输栈顶数据到本层assign读到的id
			gen(LOD, id_var_level, id_var_address);//将本层assign中id得到的值置于栈顶，以便上一层assign中的id得到该值
		}
	}
	else {
		if (sym == SYM_BECOMES) {
			error(26);//you can only assign an value to a left_value expression
		}
	}
}


  //////////////////////////////////////////////////////////////////////
void statement(symset fsys, control_list *brk_list, control_list *ctn_list)
{									//break_list and continue_list
	int i, cx1, cx2;
	symset set1, set;
	//int break_cx = 0, continue_cx = 0;
	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk;
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		assign(fsys);//expression(fsys);
		mk = (mask*)&table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
/*
		//2018-11-24
		while (sym == SYM_IDENTIFIER) {
			k++;
			if (!(id_index[k] = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[id_index[k]].kind != ID_VARIABLE)
			{
				error(12); // Illegal assignment.
				id_index[k] = 0;
			}
			previous_sym = sym;//???之前??艿姆???
			getsym();
			now_sym = sym;//?????姆???
			if (sym == SYM_BECOMES)
			{
				getsym();
			}
			else // ????????:=??说????????expression??预?????????
			{
				k--;//????id_index???谋???????-1??
				conflict = 1;//预?????????
				sym = previous_sym;//?指?之前??艿姆???
				expression(fsys);
				//printf("here\n");
				//error(13); // ':=' expected.
			}

		}
		//
		if (conflict == 0) {
			expression(fsys);
		}
		else {
			conflict = 0;
			//expression() has been executed
		}
		{
			int id_num;
			for (id_num = k; id_num >= 0; id_num--) {//通???羌潜????????id_index???????值
				mk = (mask*)&table[id_index[id_num]];
				if (id_index[id_num]) {
					if (id_num == k) {//????????????????
						gen(STO, level - mk->level, mk->address);
					}
					else {//??弑??????????弑????
						mask* mk_previous = (mask*)&table[id_index[id_num + 1]];
						gen(LOD, level - mk_previous->level, mk_previous->address);
						gen(STO, level - mk->level, mk->address);
					}
				}
			}
		}
		//2018-11-24
*/
		/*
		mk = (mask*)&table[i];
		if (i)
		{
		gen(STO, level - mk->level, mk->address);
		}
		*/
	
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called.
			}
			getsym();
		}
	}
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_RPAREN, SYM_ELSE, SYM_THEN, SYM_NULL);
		set = uniteset(set1, fsys);
		or_expr(set);//condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys, brk_list, ctn_list);
		code[cx1].a = cx;
		//2018

		if (sym == SYM_ELSE) {
			cx2 = cx;
			gen(JMP, 0, 0);
			code[cx1].a = cx;//?????lse???ode[cx1]??
			getsym();
			statement(fsys, brk_list, ctn_list);
			code[cx2].a = cx;
		}

	}
	else if (sym == SYM_BEGIN)
	{ // block
	  //11/26


		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set, brk_list, ctn_list);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set, brk_list, ctn_list);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		control_list *new_brk_list = (control_list*)calloc(1, sizeof(control_list));
		control_list *new_ctn_list = (control_list*)calloc(1, sizeof(control_list));
		init_control_list(new_brk_list);
		init_control_list(new_ctn_list);

		control_node *p;
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		or_expr(set);//condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		set1 = createset(SYM_BREAK, SYM_CONTINUE, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set, new_brk_list, new_ctn_list);//recursion
		destroyset(set1);
		destroyset(set);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		//entrance:cx1, exit:cx
		// if (exsit_break) {
		// 	exsit_break = 0;
		// 	code[break_cx].a = cx;//exit
		// }
		// if (exsit_continue) {
		// 	exsit_continue = 0;
		// 	code[continue_cx].a = cx1;//entrance
		// }
		{		//break
			p = new_brk_list->h;
			while (p) {
				code[p->cx].a = cx;
				p = p->next;
			}
		}
		{	//continue
			p = new_ctn_list->h;
			while (p) {
				code[p->cx].a = cx1;
				p = p->next;
			}

		}
		delete_control_list(new_brk_list);
		delete_control_list(new_ctn_list);

	}
	else if (sym == SYM_BREAK) {
		control_node *p = (control_node*)calloc(1, sizeof(control_node));

		getsym();
		p->cx = cx;
		add_item_control_list(brk_list, p);
		gen(JMP, 0, 0);
	}
	else if (sym == SYM_CONTINUE) {
		control_node *p = (control_node*)calloc(1, sizeof(control_node));
		getsym();
		p->cx = cx;
		add_item_control_list(ctn_list, p);
		gen(JMP, 0, 0);
	}
	test(fsys, phi, 19);
} // statement

  //////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;
	control_list* brk_list = (control_list*)calloc(1, sizeof(control_list));
	control_list* ctn_list = (control_list*)calloc(1, sizeof(control_list));
	init_control_list(brk_list);
	init_control_list(ctn_list);


	dx = 3;
	block_dx = dx;
	mk = (mask*)&table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_ELSE, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set, brk_list, ctn_list);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
	//11/26
	delete_control_list(brk_list);
	delete_control_list(ctn_list);

} // block

  //////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{   //stack -- rutime stack storing ARs
	//currentLevel -- base address for current procedure(caller) AR
	//levelDiff(non-negative) -- level difference between a name's reference point and definition point!
	//thus, when finished, we should just reach an  outside surrounding scope, getting its AR base address as return value
	//in further, if a name @levelDiff is a common variable, we could find its value within that AR.
	//if a name@levelDiff means a procedure call, we lastly stop at its parent procedure's AR, a static link could be set up!

	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

  //////////////////////////////////////////////////////////////////////
  // interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;//restore stack pointer, then free all 3 items allocated to callee's AR
				pc = stack[top + 3]; //restore ip
				b = stack[top + 2];// restore base pointer for caller's AR
				break;
			case OPR_NOT: //2017-09-24
				stack[top] = !stack[top];
				break;
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			// generate callee's AR at top of runtime stack!
			stack[top + 1] = base(stack, b, i.l);//set up SL, static link
			stack[top + 2] = b; //DL, dynamic link, saving base address for caller's AR
			stack[top + 3] = pc; //save return address,next instruction after CAL
			b = top + 1; // new base poniter points to SL
			pc = i.a;    // reset ip
						 //notice, when CAL executing, the top of stack is not changed
			break;
		case INT:
			top += i.a;//in general, as the first instruction of a function
					   //to allocate all AR space, including SL, DL, RA, variables
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

  //////////////////////////////////////////////////////////////////////
int main()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL, SYM_IDENTIFIER, SYM_ELSE);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_ODD, SYM_NOT, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
	system("pause");
} // main

  //////////////////////////////////////////////////////////////////////
  // eof pl0.c

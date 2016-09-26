%{
/* $%BEGINLICENSE%$
 Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; version 2 of the
 License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA

 $%ENDLICENSE%$ */


#include <string.h>

#include "sqlparser.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <io.h>  /* for read */
#endif
#include <stdlib.h>

#define YY_DECL int sql_tokenizer_internal(SqlParser* result, yyscan_t yyscanner)

#define GE_STR_LITERAL_WITH_LEN(str) str, sizeof(str) - 1
%}

%option reentrant
%option case-insensitive
%option noyywrap
%option never-interactive

%option noinput
%option nounput

%option 8bit
%option fast
%option nounistd
%x COMMENT LINECOMMENT QUOTED
%%
%{
    char quote_char = 0;
    sql_token_id quote_token_id = TK_UNKNOWN;
    sql_token_id comment_token_id = TK_UNKNOWN;
%}

"--"\r?\n   comment_token_id = TK_COMMENT;       result->appendToken(comment_token_id, GE_STR_LITERAL_WITH_LEN(""));
"/*"		comment_token_id = TK_COMMENT;       result->appendToken(comment_token_id, GE_STR_LITERAL_WITH_LEN("")); BEGIN(COMMENT);
"/*!"		comment_token_id = TK_COMMENT_MYSQL; result->appendToken(comment_token_id, GE_STR_LITERAL_WITH_LEN("")); BEGIN(COMMENT);
"--"		comment_token_id = TK_COMMENT;		 result->appendToken(comment_token_id, GE_STR_LITERAL_WITH_LEN("")); BEGIN(LINECOMMENT);
<COMMENT>[^*]*	result->appendTokenToLast(yytext, yyleng);
<COMMENT>"*"+[^*/]*	result->appendTokenToLast(yytext, yyleng);
<COMMENT>"*"+"/"	BEGIN(INITIAL);
<COMMENT><<EOF>>	BEGIN(INITIAL);
<LINECOMMENT>[^\n]* result->appendTokenToLast(yytext, yyleng);
<LINECOMMENT>\r?\n	BEGIN(INITIAL);
<LINECOMMENT><<EOF>>	BEGIN(INITIAL);

["'`]		{ BEGIN(QUOTED);  
		quote_char = *yytext; 
		switch (quote_char) { 
		case '\'': quote_token_id = TK_STRING; break; 
		case '"': quote_token_id = TK_LITERAL; break; 
		case '`': quote_token_id = TK_LITERAL; break; 
		} 
		result->appendToken(quote_token_id, GE_STR_LITERAL_WITH_LEN("")); }

<QUOTED>"\\'"      yymore(); /** add by huih@20160204 */
<QUOTED>[^"'`\\]*	result->appendTokenToLast(yytext, yyleng); /** all non quote or esc chars are passed through */
<QUOTED>"\\".		result->appendTokenToLast(yytext, yyleng); /** add escaping */
<QUOTED>["'`]{2}	{ if (yytext[0] == yytext[1] && yytext[1] == quote_char) { 
                result->appendTokenToLast(yytext, yyleng);  /** doubling quotes */
			} else {
				/** pick the first char and put the second back to parsing */
				yyless(1);
				result->appendTokenToLast(yytext, yyleng);
			}
			}
<QUOTED>["'`]	if (*yytext == quote_char) { BEGIN(INITIAL); } else { result->appendTokenToLast(yytext, yyleng); }
<QUOTED><<EOF>>	BEGIN(INITIAL);

	/** strings, quoting, literals */
[[:space:]]+	/** ignore WS */

	/* unquoted literals (and function names) are
	 * 
	 *   all alpha-nums that are not digits-only and NOT floats
	 *
	 * Floats are
	 *   1.1
	 *   1e+1
	 *   1.1e+1
	 *   .1e+1
	 * unquoted literals:
	 *   e1
	 *   1e
	 * complex cases
	 *   1e + 1 is a literal ("1e"), a plus ("+") and integer ("1")
	 *   1e+1e  is a float ("1e+1") and a literal ("e")
	 *   compare this to 1.1e which is INVALID (a broken scientific notation)
	 */
([[:digit:]]*".")?[[:digit:]]+[eE][-+]?[[:digit:]]+		result->appendToken(TK_FLOAT, yytext, yyleng);
"-"([[:digit:]]*".")?[[:digit:]]+[eE][-+]?[[:digit:]]+  result->appendToken(TK_FLOAT, yytext, yyleng);
"+"([[:digit:]]*".")?[[:digit:]]+[eE][-+]?[[:digit:]]+  result->appendToken(TK_FLOAT, yytext, yyleng);
	/* literals
	 * - be greedy and capture specifiers made up of up to 3 literals: lit.lit.lit
	 * - if it has a dot, split it into 3 tokens: lit dot lit
	 *
	 * when it comes to dots in specifiers spaces matter:
	 *   e1 . 1e + 1
	 *   e1.1e + 1
	 *   e1.1e+1 are all a literal ("e1"), a dot, a literal ("1e"), a plus and a integer ("1")
	 * but 
	 *   e1. 1e+1 is invalid as it is a literal ("e1"), a dot and a float ("1e+1")
	 */
[[:alpha:]_@$#][[:alnum:]_@$#]*("."[[:digit:]]*[[:alpha:]_@][[:alnum:]_@]*){0,2}	{
		char *cur, *tk_start = yytext;
		int tk_len;

		for (cur = yytext; cur < yytext + yyleng; cur++) {
			if (*cur == '.') {
				tk_len = cur - tk_start;

				result->appendToken(SqlParser::tokenIdByName(tk_start, tk_len), tk_start, tk_len);
                result->appendToken(TK_DOT, GE_STR_LITERAL_WITH_LEN("."));
				tk_start = cur + 1;
			}
		}
		/* copy the rest */
		tk_len = yytext + yyleng - tk_start;
		result->appendToken(SqlParser::tokenIdByName(tk_start, tk_len), tk_start, tk_len);
	}
[[:digit:]]+					result->appendToken(TK_INTEGER, yytext, yyleng);
"-"[[:digit:]]+					result->appendToken(TK_INTEGER, yytext, yyleng);
"+"[[:digit:]]+					result->appendToken(TK_INTEGER, yytext, yyleng);
[[:digit:]]*"."[[:digit:]]+		result->appendToken(TK_FLOAT, yytext, yyleng);
"-"[[:digit:]]*"."[[:digit:]]+  result->appendToken(TK_FLOAT, yytext, yyleng);
"+"[[:digit:]]*"."[[:digit:]]+  result->appendToken(TK_FLOAT, yytext, yyleng);
","								result->appendToken(TK_COMMA, yytext, yyleng);
"."								result->appendToken(TK_DOT, yytext, yyleng);
"<"								result->appendToken(TK_LT, yytext, yyleng);
">"								result->appendToken(TK_GT, yytext, yyleng);
"<="							result->appendToken(TK_LE, yytext, yyleng);
">="							result->appendToken(TK_GE, yytext, yyleng);
"="								result->appendToken(TK_EQ, yytext, yyleng);
"<>"							result->appendToken(TK_NE, yytext, yyleng);
"!="							result->appendToken(TK_NE, yytext, yyleng);
"("								result->appendToken(TK_OBRACE, yytext, yyleng);
")"								result->appendToken(TK_CBRACE, yytext, yyleng);
";"								//old: sql_append_remailSqlList(tokens, remailSqlList);
":="							result->appendToken(TK_ASSIGN, yytext, yyleng);
"*"								result->appendToken(TK_STAR, yytext, yyleng);
"+"								result->appendToken(TK_PLUS, yytext, yyleng);
"/"								result->appendToken(TK_DIV, yytext, yyleng);
"-"								result->appendToken(TK_MINUS, yytext, yyleng);
"&"								result->appendToken(TK_BITWISE_AND, yytext, yyleng);
"&&"							result->appendToken(TK_LOGICAL_AND, yytext, yyleng);
"|"								result->appendToken(TK_BITWISE_OR, yytext, yyleng);
"||"							result->appendToken(TK_LOGICAL_OR, yytext, yyleng);
"^"								result->appendToken(TK_BITWISE_XOR, yytext, yyleng);
	/*"$"[[:digit:]]+					//old: sql_token_replace_append_len(tokens, values, TK_STRING, yytext, yyleng); */
"::"							result->appendToken(TK_SQL_ACTIONSCOPE, yytext, yyleng);
"~"								result->appendToken(TK_SQL_MATCHREGEXCASE, yytext, yyleng);
"~*"							result->appendToken(TK_SQL_MATCHREGEXNOCASE, yytext, yyleng);
"!~"							result->appendToken(TK_SQL_NOMATCHREGEXCASE, yytext, yyleng);
"!~*"							result->appendToken(TK_SQL_NOMATCHREGEXNOCASE, yytext, yyleng);

	/** the default rule */
.								result->appendToken(TK_UNKNOWN, yytext, yyleng);

%%

int SqlParser::parse(const char *str, int len, SqlScanResults *result)
{
    YY_BUFFER_STATE state;
    yyscan_t yyscaner = NULL;
    int ret;

    yylex_init(&yyscaner);
    state = yy_scan_bytes(str, len, yyscaner);
    ret = sql_tokenizer_internal(result, yyscaner);
    yy_delete_buffer(state,yyscaner);
    yylex_destroy(yyscaner);

    return ret;
}

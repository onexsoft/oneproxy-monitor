/* $%BEGINLICENSE%$
 Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.

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
#include <strings.h>
#include <map>
#include <algorithm>

#include "sqlparser.h"

#include "../util/logger.h"
#include "hitcache.h"

#define S(x) { x, #x, sizeof(#x) - 1 }
#define cmptk(index, tkid) (token(index)->token_id == tkid)

/**
 * this list has to be kept in sync with the tokens itself
 *
 * maps token_ids (array-pos) to token-names
 */

struct TokenInfo {
	int id;
    const char* text;
    int len;
};

static TokenInfo tokenInfos[] = {
    S(TK_UNKNOWN),
    S(TK_LE),
    S(TK_GE),
    S(TK_LT),
    S(TK_GT),
    S(TK_EQ),
    S(TK_NE),
    S(TK_STRING),
    S(TK_COMMENT),
    S(TK_LITERAL),
//    S(TK_FUNCTION),

    S(TK_INTEGER),
    S(TK_FLOAT),
    S(TK_DOT),
    S(TK_COMMA),
    S(TK_ASSIGN),
    S(TK_OBRACE),
    S(TK_CBRACE),
    S(TK_SEMICOLON),
    S(TK_STAR),
    S(TK_PLUS),
    S(TK_MINUS),
    S(TK_DIV),
//
    S(TK_BITWISE_AND),
    S(TK_BITWISE_OR),
    S(TK_BITWISE_XOR),
    S(TK_LOGICAL_AND),
    S(TK_LOGICAL_OR),

    S(TK_SQL_ACCESSIBLE),
    S(TK_SQL_ACTION),
//    S(TK_SQL_ADD),
    S(TK_SQL_ALL),
    S(TK_SQL_ALTER),
//    S(TK_SQL_ANALYZE),
//    S(TK_SQL_AND),
    S(TK_SQL_AS),
//    S(TK_SQL_ASC),
//    S(TK_SQL_ASENSITIVE),
//    S(TK_SQL_BEFORE),
    S(TK_SQL_BEGIN),
//    S(TK_SQL_BETWEEN),
//    S(TK_SQL_BIGINT),
//    S(TK_SQL_BINARY),
//    S(TK_SQL_BIT),
//    S(TK_SQL_BLOB),
//    S(TK_SQL_BOTH),
//    S(TK_SQL_BY),
//    S(TK_SQL_CALL),
//    S(TK_SQL_CASCADE),
//    S(TK_SQL_CASE),
//    S(TK_SQL_CHANGE),
//    S(TK_SQL_CHAR),
//    S(TK_SQL_CHARACTER),
//    S(TK_SQL_CHECK),
//    S(TK_SQL_COLLATE),
    S(TK_SQL_COLUMN),
    S(TK_SQL_COLUMNS),
    S(TK_SQL_COMMIT),
//    S(TK_SQL_CONDITION),
//    S(TK_SQL_CONSTRAINT),
//    S(TK_SQL_CONTINUE),
//    S(TK_SQL_CONVERT),
    S(TK_SQL_CREATE),
    S(TK_SQL_CROSS),
//    S(TK_SQL_CURRENT_DATE),
//    S(TK_SQL_CURRENT_TIME),
//    S(TK_SQL_CURRENT_TIMESTAMP),
//    S(TK_SQL_CURRENT_USER),
//    S(TK_SQL_CURSOR),
//    S(TK_SQL_DATABASE),
//    S(TK_SQL_DATABASES),
//    S(TK_SQL_DATE),
//    S(TK_SQL_DAY_HOUR),
//    S(TK_SQL_DAY_MICROSECOND),
//    S(TK_SQL_DAY_MINUTE),
//    S(TK_SQL_DAY_SECOND),
//    S(TK_SQL_DEALLOCATE),
    S(TK_SQL_DEC),
//    S(TK_SQL_DECIMAL),
    S(TK_SQL_DECLARE),
//    S(TK_SQL_DEFAULT),
//    S(TK_SQL_DELAYED),
    S(TK_SQL_DELETE),
    S(TK_SQL_DESC),
    S(TK_SQL_DESCRIBE),
//    S(TK_SQL_DETERMINISTIC),
//    S(TK_SQL_DISTINCT),
//    S(TK_SQL_DISTINCTROW),
//    S(TK_SQL_DIV),
//    S(TK_SQL_DO),
//    S(TK_SQL_DOUBLE),
    S(TK_SQL_DROP),
//    S(TK_SQL_DUAL),
//    S(TK_SQL_EACH),
//    S(TK_SQL_ELSE),
//    S(TK_SQL_ELSEIF),


//    S(TK_SQL_ENCLOSED),
//    S(TK_SQL_ENUM),
//    S(TK_SQL_ESCAPED),
//    S(TK_SQL_EXEC),
//    S(TK_SQL_EXECUTE),
    S(TK_SQL_EXISTS),
//    S(TK_SQL_EXIT),
    S(TK_SQL_EXPLAIN),
//    S(TK_SQL_END), //add by huih@20151104 for pg
//    S(TK_SQL_FALSE),
//    S(TK_SQL_FETCH),
//    S(TK_SQL_FLOAT),
//    S(TK_SQL_FLOAT4),
//    S(TK_SQL_FLOAT8),
    S(TK_SQL_FOR),
    S(TK_SQL_FORCE),
//    S(TK_SQL_FOREIGN),
    S(TK_SQL_FROM),


//    S(TK_SQL_FULLTEXT),
//    S(TK_SQL_GLOBAL),
    S(TK_SQL_GRANT),
    S(TK_SQL_GROUP),
    S(TK_SQL_HANDLER),
    S(TK_SQL_HAVING),
//    S(TK_SQL_HIGH_PRIORITY),
//    S(TK_SQL_HOUR_MICROSECOND),
//    S(TK_SQL_HOUR_MINUTE),
//    S(TK_SQL_HOUR_SECOND),
    S(TK_SQL_IF),
    S(TK_SQL_IGNORE),
    S(TK_SQL_IN),
//    S(TK_SQL_INDEX),
//    S(TK_SQL_INFILE),
    S(TK_SQL_INNER),
//    S(TK_SQL_INOUT),
//    S(TK_SQL_INSENSITIVE),
    S(TK_SQL_INSERT),
//    S(TK_SQL_INT),
//    S(TK_SQL_INT1),
//    S(TK_SQL_INT2),
//    S(TK_SQL_INT3),
//    S(TK_SQL_INT4),
//    S(TK_SQL_INT8),
//    S(TK_SQL_INTEGER),
//    S(TK_SQL_INTERVAL),
    S(TK_SQL_INTO),
//    S(TK_SQL_IS),
//    S(TK_SQL_ITERATE),
    S(TK_SQL_JOIN),
//    S(TK_SQL_KEY),
//    S(TK_SQL_KEYS),
//    S(TK_SQL_KILL),
//    S(TK_SQL_LEADING),
//    S(TK_SQL_LEAVE),
    S(TK_SQL_LEFT),
//    S(TK_SQL_LIKE),
    S(TK_SQL_LIMIT),
//    S(TK_SQL_LINEAR),
//    S(TK_SQL_LINES),
//    S(TK_SQL_LOAD),
//    S(TK_SQL_LOCALTIME),
//    S(TK_SQL_LOCALTIMESTAMP),
    S(TK_SQL_LOCK),
//    S(TK_SQL_LONG),
//    S(TK_SQL_LONGBLOB),
//    S(TK_SQL_LONGTEXT),
//    S(TK_SQL_LOOP),
//    S(TK_SQL_LOW_PRIORITY),
//    S(TK_SQL_MASTER_SSL_VERIFY_SERVER_CERT),
    S(TK_SQL_MATCH),
//    S(TK_SQL_MEDIUMBLOB),
//    S(TK_SQL_MEDIUMINT),
//    S(TK_SQL_MEDIUMTEXT),
//    S(TK_SQL_MIDDLEINT),
//    S(TK_SQL_MINUTE_MICROSECOND),
//    S(TK_SQL_MINUTE_SECOND),
//    S(TK_SQL_MOD),
//    S(TK_SQL_MODIFIES),
    S(TK_SQL_NATURAL),
    S(TK_SQL_NO),
    S(TK_SQL_NOT),
//    S(TK_SQL_NO_WRITE_TO_BINLOG),
//    S(TK_SQL_NULL),


//    S(TK_SQL_NUMERIC),
    S(TK_SQL_ON),
//    S(TK_SQL_OPTIMIZE),
//    S(TK_SQL_OPTION),
//    S(TK_SQL_OPTIONALLY),
    S(TK_SQL_OR),
    S(TK_SQL_ORDER),
//    S(TK_SQL_OUT),
//    S(TK_SQL_OUTER),
//    S(TK_SQL_OUTFILE),
	S(TK_SQL_OFF),
//    S(TK_SQL_PRECISION),
//    S(TK_SQL_PREPARE),
//    S(TK_SQL_PRIMARY),
    S(TK_SQL_PROCEDURE),
//    S(TK_SQL_PURGE),
//    S(TK_SQL_RANGE),
//    S(TK_SQL_READ),
//    S(TK_SQL_READ_ONLY),
//    S(TK_SQL_READS),
//    S(TK_SQL_READ_WRITE),
//    S(TK_SQL_REAL),
//    S(TK_SQL_REFERENCES),
//    S(TK_SQL_REGEXP),
//    S(TK_SQL_RELEASE),
//    S(TK_SQL_RENAME),
//    S(TK_SQL_REPEAT),
    S(TK_SQL_REPLACE),
//    S(TK_SQL_REQUIRE),
//    S(TK_SQL_RESTRICT),
//    S(TK_SQL_RETURN),
//    S(TK_SQL_REVOKE),
    S(TK_SQL_RIGHT),
//    S(TK_SQL_RLIKE),
    S(TK_SQL_ROLLBACK),
//    S(TK_SQL_SCHEMA),
//    S(TK_SQL_SCHEMAS),
//    S(TK_SQL_SECOND_MICROSECOND),
    S(TK_SQL_SELECT),
//    S(TK_SQL_SENSITIVE),
//    S(TK_SQL_SEPARATOR),
    S(TK_SQL_SET),
    S(TK_SQL_SHOW),
//    S(TK_SQL_SMALLINT),
//    S(TK_SQL_SPATIAL),
//    S(TK_SQL_SPECIFIC),
//    S(TK_SQL_SQL),
//    S(TK_SQL_SQL_BIG_RESULT),
//    S(TK_SQL_SQL_CALC_FOUND_ROWS),
//    S(TK_SQL_SQLEXCEPTION),
//    S(TK_SQL_SQL_SMALL_RESULT),
//    S(TK_SQL_SQLSTATE),
//    S(TK_SQL_SQLWARNING),
//    S(TK_SQL_SSL),
    S(TK_SQL_START),
//    S(TK_SQL_STARTING),
    S(TK_SQL_STRAIGHT_JOIN),
    S(TK_SQL_TABLE),
//    S(TK_SQL_TERMINATED),
//    S(TK_SQL_TEXT),
//    S(TK_SQL_THEN),
//    S(TK_SQL_TIME),
//    S(TK_SQL_TIMESTAMP),
//    S(TK_SQL_TINYBLOB),
//    S(TK_SQL_TINYINT),
//    S(TK_SQL_TINYTEXT),
//    S(TK_SQL_TO),
//    S(TK_SQL_TRAILING),
    S(TK_SQL_TRANSACTION),
	S(TK_SQL_TRAN),
//    S(TK_SQL_TRIGGER),
//    S(TK_SQL_TRUE),
    S(TK_SQL_TRUNCATE),
//    S(TK_SQL_UNDO),
    S(TK_SQL_UNION),
//    S(TK_SQL_UNIQUE),
//    S(TK_SQL_UNLOCK),
//    S(TK_SQL_UNSIGNED),
    S(TK_SQL_UPDATE),
//    S(TK_SQL_USAGE),
    S(TK_SQL_USE),
    S(TK_SQL_USING),
//    S(TK_SQL_UTC_DATE),
//    S(TK_SQL_UTC_TIME),
//    S(TK_SQL_UTC_TIMESTAMP),
    S(TK_SQL_VALUE),
    S(TK_SQL_VALUES),
//    S(TK_SQL_VARBINARY),
//    S(TK_SQL_VARCHAR),
//    S(TK_SQL_VARCHARACTER),
//    S(TK_SQL_VARYING),
    S(TK_SQL_VACUUM),//add by huih@20151104 for pg
//    S(TK_SQL_WHEN),
    S(TK_SQL_WHERE),
//    S(TK_SQL_WHILE),
//    S(TK_SQL_WITH),
//    S(TK_SQL_WRITE),
//    S(TK_SQL_X509),
//    S(TK_SQL_XA),
//    S(TK_SQL_XOR),
//    S(TK_SQL_YEAR_MONTH),
//    S(TK_SQL_PARTITION),
//    S(TK_SQL_RETURNING), //add by huih@20151230, for pg
    S(TK_SQL_ACTIONSCOPE), //add by huih@20160118, for pg
    S(TK_SQL_MATCHREGEXCASE), //add by huih@20160201 for pg
    S(TK_SQL_MATCHREGEXNOCASE), //add by huih@20160201 for pg
    S(TK_SQL_NOMATCHREGEXCASE), //add by huih@20160201 for pg
    S(TK_SQL_NOMATCHREGEXNOCASE), //add by huih@20160201 for pg
//    S(TK_SQL_DISCARD), //add by huih@20160203 for pg
//    S(TK_SQL_LISTEN), //add by huih@20160203 for pg
//    S(TK_SQL_COMMENT), //add by huih@20160204 for pg
//    S(TK_SQL_NOTIFY), //add by huih@20160204 for pg
//    S(TK_SQL_SAVEPOINT), //add by huih@20160204 for pg
	S(TK_SQL_IMPLICIT_TRANSACTIONS),
    S(TK_SQL_ZEROFILL),

    S(TK_COMMENT_MYSQL),

    { -1, NULL, 0 }
};
#undef S

struct CmpToken{
	bool operator() (const std::string& a, const std::string& b) const {
		if (strcasecmp(a.c_str(), b.c_str()) < 0) {
			return true;
		}
		return false;
	}
};

class TokenMap
{
public:
    TokenMap(void):hitCache(2, true) {
    	int i = 0;
    	while(tokenInfos[i].id != TK_SQL_ACCESSIBLE) i++;
    	while(tokenInfos[i].id != TK_SQL_ZEROFILL && tokenInfos[i].id != -1) {
    		if (tokenInfos[i].len > 7) {
    			std::string s(tokenInfos[i].text + 7, tokenInfos[i].len - 7);
    			token_map.insert(tokenmap_t::value_type(s, tokenInfos[i].id));
    			hitCache.set_hitCache(tokenInfos[i].text + 7, tokenInfos[i].len - 7);
    		}
    		i = i + 1;
    	}
    }
    ~TokenMap(void) {}

    sql_token_id tokenIdByName(const char* name, int len)
    {
    	if (hitCache.is_hit(name, len)) {
			tokenmap_t::iterator it = token_map.find(std::string(name, len));
			if (it != token_map.end()) {
				return (sql_token_id)it->second;
			}
    	}
        return TK_LITERAL;
    }

    typedef std::map<std::string, int, CmpToken> tokenmap_t;
    tokenmap_t token_map;
    HitCache hitCache;
};

static TokenMap tokenMap;

sql_token_id sql_token_is_literal(const char* name, int len)
{
    return tokenMap.tokenIdByName(name, len);
}

SqlParser::SqlParser(void)
{
    m_local_begin = 0;
    m_local_end = 0;
    this->m_isBigQuery = false;
    this->m_is_insert_select = false;
}

SqlParser::~SqlParser(void)
{
	if (this->m_tokenCache.size() > 0) {
		std::list<sql_token*>::iterator it = m_tokenCache.begin();
		for (; it != m_tokenCache.end();) {
			sql_token* token = *it;
			delete token;
			++it;
		}
	}

	if (this->m_tokens.size() > 0) {
		std::vector<sql_token*>::iterator it = this->m_tokens.begin();
		for (; it != this->m_tokens.end(); ) {
			sql_token* token = *it;
			delete token;
			++it;
		}
	}
}

bool SqlParser::parse(const char *str, int len)
{
    reset();

    int ret = parseToken(str, len);
    if (ret != 0) {
        return false;
    }

    m_local_end = tokenCount();
//    this->show_tokens(m_local_begin, m_local_end);

    parseSubQuery(m_local_begin, m_local_end);

    for (unsigned int i = 0; i < m_querylist.size(); ++i) {
        SQLQuery* query = &m_querylist[i];
        parseQueryTable(query);
    }
    return true;
}

void SqlParser::reset(void)
{
    m_local_begin = 0;
    m_local_end   = 0;
    m_isBigQuery = false;
    m_is_insert_select = false;
    m_tablelist.clear();
    m_querylist.clear();

    for (std::vector<sql_token*>::size_type i = 0; i < m_tokens.size(); ++i) {
    	m_tokens[i]->text.clear();
    	m_tokens[i]->token_id = TK_UNKNOWN;
    	this->m_tokenCache.push_back(m_tokens[i]);
    }
    m_tokens.reserve(64);
    m_tokens.clear();
}

void SqlParser::appendToken(sql_token_id token_id, const char *text, int len)
{
    if (len == -1) {
        len = strlen(text);
    }
    sql_token *token = getSqlTokenFromCache();
    if (token == NULL) {
    	logs(Logger::ERR, "get sql token from cache error");
    }

    token->token_id = TK_UNKNOWN;
    sql_token *last_token = NULL;

    if (!m_tokens.empty()) {
        last_token = m_tokens.back();
    }

    token->token_id = token_id;
    if ((token_id == TK_FLOAT || token_id == TK_INTEGER) &&
            (text[0] == '-' || text[0] == '+') &&
            (last_token && (last_token->token_id == TK_LITERAL ||
                            last_token->token_id == TK_FLOAT ||
                            last_token->token_id == TK_INTEGER ||
                            last_token->token_id == TK_STRING))) {
        if (text[0] == '-')	{
            appendToken(TK_MINUS, "-", 1);
        } else {
            appendToken(TK_PLUS, "+", 1);
        }
        token->text.append(text + 1, len - 1);
    } else {
        token->text.append(text, len);
    }

    m_tokens.push_back(token);
}

void SqlParser::appendTokenToLast(const char *text, int len)
{
    if (!m_tokens.empty()) {
        if (len == -1) {
            len = strlen(text);
        }
        sql_token* token = m_tokens.back();
        token->text.append(text, len);
    }
}

void SqlParser::toPatternQuery(std::string &s)
{
    s.clear();
    for (int i = 0; i < tokenCount(); ++i) {
        sql_token* t = token(i);
        sql_token_id token_id = t->token_id;
        if (i > 0) {
            sql_token* prev = token(i - 1);
            if (prev->token_id != TK_DOT && token_id != TK_DOT) {
                if (prev->token_id == TK_SQL_FROM ||
                        prev->token_id == TK_SQL_WHERE ||
                        prev->token_id == TK_SQL_IN ||
                        prev->token_id == TK_SQL_EXISTS ||
                        prev->token_id == TK_SQL_SELECT ||
                        (token_id != TK_CBRACE && token_id != TK_OBRACE && prev->token_id != TK_OBRACE))
                    s.append(" ", 1);
            }
        }
        if (token_id == TK_STRING) {
            s.append("?", 1);
        } else if (token_id == TK_INTEGER) {
            s.append("?", 1);
        } else if (token_id == TK_FLOAT) {
            s.append("?", 1);
        } else if (token_id == TK_COMMENT) {
            s.append("/*", 2);
            s.append("*/", 2);
        } else if (token_id == TK_COMMENT_MYSQL) {
            s.append("/*!", 3);
            s.append("*/", 2);
        } else {
            s.append(t->text.data(), t->text.size());
        }
    }
}

int SqlParser::queryType(void)
{
    int tkstart = skipExternTokensInHeader(m_local_begin, m_local_end);
    if (tkstart == m_local_end) {
        return Unknown;
    }

    switch (token(tkstart)->token_id) {
    case TK_SQL_SELECT:
        return Select;
    case TK_SQL_INSERT:
        return Insert;
    case TK_SQL_REPLACE:
        return Replace;
    case TK_SQL_UPDATE:
        return Update;
    case TK_SQL_DELETE:
        return Delete;
    case TK_SQL_COMMIT:
        return Commit;
    case TK_SQL_ROLLBACK:
        return Rollback;
    default:
        break;
    }
    return Unknown;
}

bool SqlParser::isDDLQuery(void)
{
    int tkstart = skipExternTokensInHeader(m_local_begin, m_local_end);
    if (tkstart == m_local_end) {
        return false;
    }

    switch (token(tkstart)->token_id)
    {
    case TK_SQL_CREATE:
    case TK_SQL_ALTER:
    case TK_SQL_DROP:
    case TK_SQL_TRUNCATE:
    case TK_SQL_DESCRIBE:
    case TK_SQL_DESC:
        return true;
    default:
        break;
    }
    return false;
}

bool SqlParser::isDCLQuery(void)
{
    int tkstart = skipExternTokensInHeader(m_local_begin, m_local_end);
    if (tkstart == m_local_end) {
        return false;
    }

    switch (token(tkstart)->token_id)
    {
    case TK_SQL_GRANT:
        return true;
    default:
        break;
    }
    return false;
}

sql_token_id SqlParser::tokenIdByName(const char *name, int len)
{
    return sql_token_is_literal(name, len);
}

sql_token_id SqlParser::tokenIsLiteral(const char *name, int len)
{
    return sql_token_is_literal(name, len);
}

bool SqlParser::parseTableNameAndAlias(SQLQuery *query, int start, int end)
{
    for (int i = start; i < end;) {
        // Assume ',' separated mutiple table list
        // this is not always true

        // find the first table part
        int keyIds[] = {TK_SQL_INNER, TK_SQL_CROSS, TK_SQL_STRAIGHT_JOIN,
                        TK_SQL_LEFT, TK_SQL_RIGHT, TK_SQL_NATURAL, TK_SQL_JOIN, TK_COMMA};
        int table0End;
        if (findTokens(i, end, keyIds, sizeof(keyIds) / sizeof(int), &table0End)) {
            if (table0End - i > 5) {
                if (token(table0End)->token_id == TK_COMMA)
                    i = table0End + 1;
                else
                    i = table0End;
                continue;
            }
            parseSimpleTableNameAndAlias(query, i, table0End);
            if (token(table0End)->token_id == TK_COMMA)
                i = table0End + 1;
            else
                i = table0End;
        } else {
            return parseSimpleTableNameAndAlias(query, i, table0End);
        }

        bool firstJoin = true;
        while (1) {
            // find the second table in join
            int joinKw[] = {TK_SQL_JOIN, TK_SQL_STRAIGHT_JOIN, TK_COMMA};
            int table1Start;
            if (!findTokens(table0End, end, joinKw, sizeof(joinKw)/sizeof(int), &table1Start)) {
                if (token(table0End - 1)->token_id == TK_COMMA && table1Start - table0End  < 5) {
                    parseSimpleTableNameAndAlias(query, table0End, table1Start);
                }
                return !firstJoin;
            }

            if (token(table1Start)->token_id == TK_COMMA) {
                if (table1Start - table0End < 5) {
                    parseSimpleTableNameAndAlias(query, table0End, table1Start);
                }
                table0End = table1Start + 1;
                firstJoin = false;
                continue;
            }

            table1Start ++;

            int table1End;
            int condKw[] = {TK_SQL_ON, TK_SQL_USING, TK_COMMA};
            bool found = findTokens(table1Start, end, condKw, sizeof(condKw)/sizeof(int), &table1End);

            if (table1End - table1Start > 5) {
                i = table1End + 1;
                table0End = i;
                firstJoin = false;
                continue;
            }
            parseSimpleTableNameAndAlias(query, table1Start, table1End);

            i = table1End + 1;
            table0End = i;
            firstJoin = false;

            if (found) {
                continue;
            } else {
                return true;
            }
        }
    }
    return true;
}

bool SqlParser::parseSimpleTableNameAndAlias(SQLQuery *query, int start, int end)
{
    //sql_token** ts = (sql_token**)(tokens->tokens->pdata);

    //add by huih@20151013, remove some sql contains SEMICOLON
    while(end > start && token(end - 1)->token_id == TK_SEMICOLON)
        --end;
    if (end <= start) {
        return false;
    }

//    {
//    	logs(Logger::ERR, "end: %d, start: %d", end, start);
//		int i = 0;
//		for (i = start; i < end; ++i) {
//			logs(Logger::ERR, "tokenid: %d", token(i)->token_id);
//		}
//    }

    if ((end - start) == 1) {//only contains table name
        if (token(start)->token_id == TK_LITERAL) {
            addTableInfo(token(start)->text, std::string(), start, query->tbegin, query->tend,
                         query->fromstart, query->fromend, query->wherestart, query->whereend,
                         m_isBigQuery);
            return true;
        }
    } else if (end - start == 2) {//format: tablename tableAliasName
        if (token(start)->token_id == TK_LITERAL && token(start + 1)->token_id == TK_LITERAL) {
            addTableInfo(token(start + 0)->text, token(start + 1)->text, start, query->tbegin,
                         query->tend, query->fromstart, query->fromend, query->wherestart,
                         query->whereend, m_isBigQuery);

            return true;
        }
    } else if (end - start == 3 && token(start+1)->token_id == TK_SQL_AS) {
        if (token(start)->token_id == TK_LITERAL && token(start + 2)->token_id == TK_LITERAL) {//format: tableName as tableAliasName
            addTableInfo(token(start + 0)->text, token(start + 2)->text, start,
                         query->tbegin, query->tend, query->fromstart, query->fromend,
                         query->wherestart, query->whereend, m_isBigQuery);
            return true;
        }
    } else if (end - start == 3 && token(start+1)->token_id == TK_DOT) {
        if (token(start)->token_id == TK_LITERAL && token(start + 2)->token_id == TK_LITERAL) {//format: namespaceName.tableName
            addTableInfo(token(start + 2)->text, std::string(), start + 2,
                         query->tbegin, query->tend, query->fromstart, query->fromend,
                         query->wherestart, query->whereend, m_isBigQuery);
            return true;
        }
    } else if (end - start == 4 && token(start+1)->token_id == TK_DOT) {
        if (token(start)->token_id == TK_LITERAL && token(start + 2)->token_id == TK_LITERAL && token(start + 3)->token_id == TK_LITERAL) {
            //format:namespace.tablename tableAliasName
            addTableInfo(token(start + 2)->text, token(start + 3)->text, start + 2,
                         query->tbegin, query->tend, query->fromstart, query->fromend,
                         query->wherestart, query->whereend, m_isBigQuery);
            return true;
        }
    } else if (end - start == 5 && token(start+1)->token_id == TK_DOT && token(start+3)->token_id == TK_SQL_AS) {
        if (token(start)->token_id == TK_LITERAL &&
                token(start + 2)->token_id == TK_LITERAL && token(start + 4)->token_id == TK_LITERAL) {
            //format is:namespace.tableName as aliasName
            addTableInfo(token(start + 2)->text, token(start + 4)->text, start + 2,
                         query->tbegin, query->tend, query->fromstart, query->fromend,
                         query->wherestart, query->whereend, m_isBigQuery);
            return true;
        }
    } else if ((end - start == 5)
    		&& cmptk(start, TK_LITERAL) && cmptk(start + 1, TK_DOT)
			&& cmptk(start + 2, TK_LITERAL) && cmptk(start + 3, TK_DOT)) {
    	//format is: database.user.tablename
    	addTableInfo(token(start + 4)->text, std::string(), start + 4,
						 query->tbegin, query->tend, query->fromstart, query->fromend,
						 query->wherestart, query->whereend, m_isBigQuery);
    	return true;

    }  else if ((end - start == 6)
    		&& cmptk(start, TK_LITERAL) && cmptk(start + 1, TK_DOT)
    		&& cmptk(start + 2, TK_LITERAL) && cmptk(start + 3, TK_DOT)
			&& cmptk(start + 5, TK_LITERAL)) {
    	//format is: database.user.tablename aliasName
    	addTableInfo(token(start + 4)->text, token(start + 5)->text, start + 4,
						 query->tbegin, query->tend, query->fromstart, query->fromend,
						 query->wherestart, query->whereend, m_isBigQuery);
    	return true;
    } else if ((end - start == 7) &&
    		cmptk(start, TK_LITERAL) && cmptk(start + 1, TK_DOT) &&
			cmptk(start + 2, TK_LITERAL) && cmptk(start + 3, TK_DOT) &&
			cmptk(start + 5, TK_SQL_AS) && cmptk(start + 6, TK_LITERAL)) {
    	//format is : master.sys.databases AS dtb
    	addTableInfo(token(start + 4)->text, token(start + 6)->text, start + 4,
						 query->tbegin, query->tend, query->fromstart, query->fromend,
						 query->wherestart, query->whereend, m_isBigQuery);
    	return true;
    }
    return false;
}

bool SqlParser::parseSubQuery(int tbegin, int tend)
{
    int union_pos = 0;
    int fromstart = 0;
    int fromend = 0;
    int tkstart = skipExternTokensInHeader(tbegin, tend);
    if (tkstart == tend) {
        return true;
    }
    tbegin = tkstart;
//    this->show_tokens(tbegin, tend);
    int tokenid = token(tbegin)->token_id;
    if (tokenid == TK_SQL_SELECT)
    {
        if (getSqlFrom(tbegin, tend, &(fromstart), &(fromend)))
        {
            SQLQuery query;
            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = fromstart;
            query.fromend    = fromend;
            query.wherestart = 0;
            query.whereend   = 0;

            if (findToken(tbegin, tend, TK_SQL_UNION, &(union_pos))) {
                query.tend = union_pos;
                getSqlWhere(query.fromend, query.tend, &(query.wherestart), &(query.whereend));
                m_querylist.push_back(query);
                ++union_pos;
                if (token(union_pos)->token_id == TK_SQL_ALL) {
                    union_pos++;
                }

                if (token(union_pos)->token_id == TK_SQL_SELECT) {
                    parseSubQuery(union_pos, tend);
                }
            } else {
                getSqlWhere(query.fromend, query.tend, &(query.wherestart), &(query.whereend));
                m_querylist.push_back(query);
            }

            // 2, get sub query from select list
            handleSubQuery(query.tbegin, query.fromstart);

            // 3, get sub query from table list
            handleSubQuery(query.fromstart, query.fromend);

            // 4, get sub query from where list
            handleSubQuery(query.wherestart, query.whereend);
        }
    }
    else if (tokenid == TK_SQL_UPDATE)
    {
        if (findToken(tbegin, tend, TK_SQL_SET, &fromstart) && token(fromstart - 1)->token_id == TK_LITERAL)
        {
            SQLQuery query;

            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = fromstart - 1;
            query.fromend    = fromstart;
            query.wherestart = 0;
            query.whereend   = 0;

            //update bigtest set age = 10 where name='test';
            if (findToken(tbegin, query.fromstart + 1, TK_LITERAL, &fromstart))
                query.fromstart = fromstart;
            else
                query.fromstart = 2;

            fromstart = query.fromend;

            findToken(fromstart, tend, TK_SQL_WHERE, &(query.wherestart));
            query.whereend   = query.wherestart;

            m_querylist.push_back(query);

            // 1, get sub query from set list
            //update bigtest set age = (select age from bigage where name='test') where name='test';
            handleSubQuery(query.fromstart, query.wherestart);

            // 2, get sub query from where list
            //update bigtest set age = 11 where name=(select name from bigage where age=30 limit 1);
            handleSubQuery(query.wherestart, query.tend);
        }
    }
    else if (tokenid == TK_SQL_DELETE)
    {
        if (findToken(tbegin, tend, TK_SQL_FROM, &fromstart) && fromstart < tend - 1)
        {
            SQLQuery query;

            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = fromstart + 1;
            query.fromend    = tend;
            query.wherestart = 0;
            query.whereend   = 0;

            findToken(fromstart + 1, tend, TK_SQL_WHERE, &(query.wherestart));
            query.fromend    = query.wherestart;
            query.whereend   = query.tend;

            m_querylist.push_back(query);

            // 1, get sub query from set list
            handleSubQuery(query.fromend, query.wherestart);

            // 2, get sub query from where list
            handleSubQuery(query.wherestart, query.tend);
        }
    }
    else if (tokenid == TK_SQL_INSERT || tokenid == TK_SQL_REPLACE)
    {
        if (findToken(tbegin, tend, TK_SQL_INTO, &fromstart) && fromstart + 1 < tend)
        {
            SQLQuery query;
            int keyIds[] = {TK_SQL_VALUES,TK_SQL_VALUE};

            // move to the token after INTO key words
            fromstart = fromstart + 1;

            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = fromstart;
            query.fromend    = fromstart;
            query.wherestart = 0;
            query.whereend   = 0;

            while(fromstart + 1 < tend
                  && token(fromstart + 1)->token_id != TK_OBRACE
                  && token(fromstart + 1)->token_id != TK_SQL_VALUES
                  && token(fromstart + 1)->token_id != TK_SQL_VALUE
                  && token(fromstart + 1)->token_id != TK_SQL_SELECT
                  && token(fromstart + 1)->token_id != TK_SQL_SET)
            {
                fromstart ++;
            }
            query.fromend    = fromstart + 1;
            fromend = fromstart + 1;

            if ((token(fromend)->token_id == TK_SQL_VALUES
                 || token(fromend)->token_id == TK_SQL_VALUE) && token(fromend + 1)->token_id == TK_OBRACE)
            {
                query.wherestart = fromend;
                query.whereend   = fromend;
            }
            else if (token(fromend)->token_id == TK_SQL_SET)
            {
                query.wherestart = fromend;
                query.whereend   = fromend;
            }
            else if (findTokens(fromend, tend, keyIds, sizeof(keyIds) / sizeof(int),&fromend) && token(fromend + 1)->token_id == TK_OBRACE)
            {
                query.wherestart = fromend;
                query.whereend   = fromend;
            }

            m_querylist.push_back(query);

            if (fromend == tend || (token(fromend)->token_id != TK_SQL_VALUES && token(fromend)->token_id != TK_SQL_VALUE))
            {
                fromend = query.fromend;
                if (findToken(fromend, tend, TK_SQL_SELECT, &fromend))
                {
                    m_is_insert_select = true;
                    parseSubQuery(fromend, tend);
                }
            }
        }
    }
    else if (tokenid == TK_SQL_CREATE || tokenid == TK_SQL_ALTER || tokenid == TK_SQL_DROP)
    {
        if (tbegin + 2 < tend && token(tbegin + 1)->token_id == TK_SQL_TABLE)
        {
            SQLQuery query;
            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = 0;
            query.fromend    = 0;
            query.wherestart = 0;
            query.whereend   = 0;
            query.fromstart = tbegin + 2;
            query.fromend   = tbegin + 3;

            // handle create table if not exists
            while(token(query.fromstart)->token_id == TK_SQL_IF ||
                  token(query.fromstart)->token_id == TK_SQL_NOT ||
                  token(query.fromstart)->token_id == TK_SQL_EXISTS)
            {
                query.fromstart ++;
                query.fromend ++;
            }

            if (query.fromend + 1 < tend && token(fromend)->token_id == TK_DOT) {
                query.fromend   = query.fromend + 2;
            }

            m_querylist.push_back(query);
        }
    }
    else if (tokenid == TK_SQL_DESCRIBE || tokenid == TK_SQL_DESC)
    {
        if (tbegin + 1 < tend && token(tbegin + 1)->token_id == TK_LITERAL) {
            SQLQuery query;
            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = 0;
            query.fromend    = 0;
            query.wherestart = 0;
            query.whereend   = 0;
            query.fromstart = tbegin + 1;
            query.fromend   = tbegin + 2;
            if (tbegin + 3 < tend && token(tbegin + 2)->token_id == TK_DOT) {
                query.fromend   = tbegin + 4;
            }
            m_querylist.push_back(query);
        }
    }
    else if (tokenid == TK_SQL_SHOW)
    {
        int poscol = 0;
        int posfrom = 0;
        if (tbegin + 2 < tend && findToken(1, tend, TK_SQL_COLUMNS, &(poscol)))
        {
            SQLQuery query;

            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = 0;
            query.fromend    = 0;
            query.wherestart = 0;
            query.whereend   = 0;

            if (findToken(poscol + 1, tend, TK_SQL_FROM, &(posfrom))) {
                query.fromstart = posfrom + 1;
                query.fromend   = posfrom + 2;

                if (posfrom + 3 < tend && token(posfrom + 2)->token_id == TK_DOT) {
                    query.fromend   = posfrom + 4;
                }
            } else {
                query.fromstart = poscol + 1;
                query.fromend   = poscol + 2;

                if (poscol + 3 < tend && token(poscol + 2)->token_id == TK_DOT) {
                    query.fromend   = poscol + 4;
                }
            }

            m_querylist.push_back(query);
        }
    }
    else if (tokenid == TK_SQL_TRUNCATE)
    {
        if (tbegin + 2 < tend && token(tbegin + 1)->token_id == TK_SQL_TABLE) {
            SQLQuery query;
            query.tbegin     = tbegin;
            query.tend       = tend;
            query.fromstart  = 0;
            query.fromend    = 0;
            query.wherestart = 0;
            query.whereend   = 0;
            query.fromstart = tbegin + 2;
            query.fromend   = tbegin + 3;

            if (tbegin + 4 < tend && token(tbegin + 3)->token_id == TK_DOT) {
                query.fromend   = tbegin + 5;
            }
            m_querylist.push_back(query);
        } else if (tbegin + 1 < tend && token(tbegin + 1)->token_id == TK_LITERAL) {
        	SQLQuery query;
        	query.tbegin = tbegin;
        	query.tend = tend;
        	query.fromstart = tbegin + 1;
        	query.fromend = tbegin + 2;
        	query.wherestart = 0;
        	query.whereend = 0;
        	m_querylist.push_back(query);
        }
    }
    else if (tokenid == TK_SQL_BEGIN)
    {
        fromstart = tbegin;
        fromend   = tbegin;

        while (fromend < tend) {
            while(fromend < tend && TK_SEMICOLON != token(fromend)->token_id)
                fromend++;
            if (fromstart > tbegin) {
                parseSubQuery(fromstart, fromend);
            }
            if (fromend < tend) {
                fromend ++;
                fromstart = fromend;
            }
        }
    }
    else if (tokenid == TK_SQL_VACUUM)
    {
    	if (tbegin + 1 < tend && token(tbegin + 1)->token_id == TK_LITERAL) {
			SQLQuery query;
			query.tbegin = tbegin;
			query.tend = tend;
			query.fromstart = tbegin + 1;
			query.fromend = tbegin + 2;
			query.wherestart = 0;
			query.whereend = 0;
			m_querylist.push_back(query);
		}
    }
    return true;
}

void SqlParser::parseQueryTable(SQLQuery *query)
{
    //sql_token** ts = (sql_token**)(tokens->tokens->pdata);
    switch (token(query->tbegin)->token_id)
    {
		case TK_SQL_SELECT:
		{
			parseTableNameAndAlias(query, query->fromstart, query->fromend);

			//parse the select column name,
			//for example: select public.bigtable.id, public.bigtable.name from public.bigtable;
			//parseSelectColumnTableName(query);

			//add by huih@20160106, parse the other part
			//parsePostgresqlTableName(query, query->fromend, query->tend);
			break;
		}
		case TK_SQL_UPDATE:
		{
			parseSimpleTableNameAndAlias(query, query->fromstart, query->fromend);

			//add by huih@20160105, for parse : update public.bigtable set age = age + 10 where public.bigtable.id=10;
			//parseWhereConditionTableName(query);

			//update public.bigtable_0 set name = 'wangshan4' where public.bigtable_0.id=10 returning public.bigtable_0.id
			//parse the returning table name, add by huih@20151230
			//parseReturningTableName(query);
			break;
		}

		case TK_SQL_INSERT:
		case TK_SQL_REPLACE:
		{
			parseSimpleTableNameAndAlias(query, query->fromstart, query->fromend);

			//insert into public.bigtable_0 (id, name, age, money)values(11,'wangshan11', 11, 1100)
			//returning public.bigtable_0.id;
			//need to parse returning
			//add by huih@20151230
			//parseReturningTableName(query);
			break;
		}
		case TK_SQL_DELETE:
		{
			parseSimpleTableNameAndAlias(query, query->fromstart, query->fromend);

			//add by huih@20160105, for parse:delete from public.bigtable where public.bigtable.id = 10;
			//parseWhereConditionTableName(query);

			//delete from public.bigtable_0 where public.bigtable_0.id = 11 returning public.bigtable_0.id;
			//need to parse returning
			//add by huih@20151230
			//parseReturningTableName(query);
			break;
		}
		case TK_SQL_CREATE:
		case TK_SQL_ALTER:
		case TK_SQL_DROP:
		{
			parseSimpleTableNameAndAlias(query, query->fromstart, query->fromend);
			break;
		}
		case TK_SQL_DESCRIBE:
		case TK_SQL_DESC:
		case TK_SQL_TRUNCATE:
		case TK_SQL_SHOW:
		case TK_SQL_VACUUM:
		{
			parseSimpleTableNameAndAlias(query, query->fromstart, query->fromend);
			break;
		}
		default:
		{
			break;
		}
    }
}

bool SqlParser::findToken(int start, int end, int tokId, int *where)
{
    int i = start;
    int brace_count = 0;

    while (i < end) {
        sql_token_id id = token(i)->token_id;
        if (brace_count != 0 || id != tokId) {
            if (id == TK_OBRACE) {
                brace_count++;
            } else if (id == TK_CBRACE) {
                brace_count--;
            }
        } else {
            break;
        }
        i++;
    }

    *where = i;
    return i < end;
}

bool SqlParser::findTokens(int start, int end, int *tokIds, int size, int *where)
{
    int i;
    int brace_count = 0;
    for (i = start; i < end; i++) {
        sql_token_id id = token(i)->token_id;
        if (id == TK_OBRACE) {
            ++brace_count;
        } else if (id == TK_CBRACE){
            --brace_count;
        } else if (brace_count == 0) {
            for (int k = 0; k < size; k++) {
                if (tokIds[k] == id) {
                    *where = i;
                    return true;
                }
            }
        }
    }
    *where = i;
    return false;
}

bool SqlParser::getSqlFrom(int begin, int tkend, int *start, int *end)
{
    if (!findToken(begin, tkend, TK_SQL_FROM, start)) {
        return false;
    }

    ++(*start);

    int ids[] = {TK_SQL_UNION, TK_SQL_WHERE, TK_SQL_GROUP, TK_SQL_HAVING, TK_SQL_ORDER,
                 TK_SQL_LIMIT, TK_SQL_PROCEDURE, TK_SQL_INTO, TK_SQL_FOR,
                 TK_SQL_LOCK, TK_SQL_USE, TK_SQL_IGNORE, TK_SQL_FORCE};

    findTokens(*start, tkend, ids, sizeof(ids)/sizeof(int), end);
    return true;
}

bool SqlParser::getSqlWhere(int begin, int tkend, int *start, int *end)
{
    if (!findToken(begin, tkend, TK_SQL_WHERE, start)) {
        m_isBigQuery = true;
        return false;
    }

    ++(*start);

    int ids[] = {TK_SQL_UNION, TK_SQL_GROUP, TK_SQL_HAVING, TK_SQL_ORDER,
                 TK_SQL_LIMIT, TK_SQL_PROCEDURE, TK_SQL_INTO, TK_SQL_FOR,
                 TK_SQL_LOCK};

    if (findTokens(*start, tkend, ids, sizeof(ids)/sizeof(int), end)) {
        switch(token(*end)->token_id)
        {
        case TK_SQL_UNION:
        case TK_SQL_GROUP:
        case TK_SQL_HAVING:
        case TK_SQL_ORDER:
            m_isBigQuery = true;
            break;
        default:
            break;
        }
    }

    return true;
}

void SqlParser::handleSubQuery(int substart, int subend)
{
    int i = 0;
    int start_brace = 0;
    int end_brace = 0;
    int brace_ount = 0;
    for (i = substart; i < subend; i++) {
        sql_token_id id = token(i)->token_id;
        if (id == TK_OBRACE) {
            if (brace_ount == 0) {
                start_brace = i;
            }
            brace_ount++;
        } else if (id == TK_CBRACE) {
            brace_ount--;
            if (brace_ount == 0) {
                end_brace = i;
                while(token(start_brace)->token_id == TK_OBRACE
                      && token(end_brace)->token_id == TK_CBRACE)
                {
                    start_brace++;
                    end_brace--;

                    if (token(start_brace)->token_id == TK_SQL_SELECT) {
                        parseSubQuery(start_brace, end_brace + 1);
                    }
                }
            }
        }
    }
}

void SqlParser::addTableInfo(const std::string &tableName,
                             const std::string &aliasName, int position, int begin, int end,
                             int fromBegin, int fromEnd, int whereBegin, int whereEnd, bool bigquery,
                             bool use_alias_method, int replaceBegin, int replaceEnd)
{
    SQLTable tab;
    strcpy(tab.table, tableName.c_str());
    strcpy(tab.alias, aliasName.c_str());
    tab.position = position;
    tab.tbegin = begin;
    tab.tend   = end;
    tab.fromstart = fromBegin;
    tab.fromend   = fromEnd;
    tab.wherestart= whereBegin;
    tab.whereend  = whereEnd;
    tab.big_query = bigquery;

    tab.use_alias_method = use_alias_method;
    tab.replacebgin = replaceBegin;
    tab.replaceend = replaceEnd;
    m_tablelist.push_back(tab);
}

int SqlParser::skipSomeHeaderTokens(int start, int end, int tokenId)
{
    while(start < end && token(start)->token_id == tokenId) {
        start++;
    }
    return start;
}

int SqlParser::skipExternTokensInHeader(int start, int end)
{
    start = skipSomeHeaderTokens(start, end, TK_COMMENT);
    start = skipSomeHeaderTokens(start, end, TK_SQL_EXPLAIN);
    return start;
}

sql_token* SqlParser::getSqlTokenFromCache()
{
	sql_token* token = NULL;
	if (this->m_tokenCache.size() > 0) {
		token = m_tokenCache.front();
		m_tokenCache.pop_front();
	} else {
		token = new sql_token();
	}
	return token;
}

int SqlParser::getQueryType(int ndx)
{
    int tkstart = m_local_begin;
    if (ndx >= 0 && ndx < (int)m_tablelist.size()) {
        SQLTable& tab = m_tablelist[ndx];
        tkstart = tab.tbegin;
    }

    if (tokenCount() > 0) {
        tkstart = skipExternTokensInHeader(tkstart, tokenCount());
        if (tkstart == tokenCount()) {
            return Unknown;
        }

        switch (token(tkstart)->token_id) {
        case TK_SQL_SELECT:
            return Select;
        case TK_SQL_INSERT:
            return Insert;
        case TK_SQL_REPLACE:
            return Replace;
        case TK_SQL_UPDATE:
            return Update;
        case TK_SQL_DELETE:
            return Delete;
        default:
            break;
        }
    }
    return Unknown;
}

const char *SqlParser::tokenNameById(sql_token_id id)
{
    if (id < 0 || id >= TK_LAST_TOKEN) {
        return "";
    }

    if (sizeof(tokenInfos) / sizeof(tokenInfos[0]) != TK_LAST_TOKEN + 1) {
        return "";
    }
    return tokenInfos[id].text;
}

bool SqlParser::isStartTrans()
{
	int i;
	sql_token* tk1 = NULL;
	sql_token* tk2 = NULL;

	int tkstart = m_local_begin;

	//add by huih@20160119
	tkstart = skipExternTokensInHeader(tkstart, m_local_end);
//    while(getTokenId(tkstart) == TK_COMMENT) tkstart++;
	if (tkstart >= m_local_end)
		return false;

	//add by huih@20160612, fixed the translation bug
	if (token(tkstart)->token_id == TK_SQL_BEGIN) {
		return true;
	}

	for(i = tkstart; i < m_local_end; i++)
	{
		if (token(i)->token_id == TK_SQL_START || token(i)->token_id == TK_SQL_TRANSACTION)
		{
			if (tk1 == NULL)
			 tk1 = token(i);
			else if (tk2 == NULL)
			 tk2 = token(i);
			else
			 break;
		}
		else
		{
			if (token(i)->token_id != TK_COMMENT)
			 break;
		}
	}
	if (tk1 && tk2)
	{
		if (tk1->token_id == TK_SQL_START && tk2->token_id == TK_SQL_TRANSACTION)
		{
			return true;
		}
	}
	return false;
}


void SqlParser::show_tokens(int start, int end)
{
	if (start <= 0)
		start = 0;
	if (end <= 0 || end > this->tokenCount())
		end = this->tokenCount();

	logs(Logger::ERR, "start: %d, end: %d", start, end);
	for (int i = start; i < end; ++i) {
		sql_token* tok = token(i);
		logs(Logger::ERR, "tokenid: %d, token: %s\n",tok->token_id, tok->text.c_str());
	}
}

/*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
* @FileName: sqlparser.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年8月15日
*
*/
#ifndef SQL_TOKENIZER_H
#define SQL_TOKENIZER_H

#include <vector>
#include <string>
#include <iostream>
#include <list>

typedef enum {
    TK_UNKNOWN,

    TK_LE,
    TK_GE,
    TK_LT,
    TK_GT,
    TK_EQ,
    TK_NE,

    TK_STRING,
    TK_COMMENT,
    TK_LITERAL,
    TK_FUNCTION,

    TK_INTEGER,
    TK_FLOAT,
    TK_DOT,
    TK_COMMA,

    TK_ASSIGN,
    TK_OBRACE,
    TK_CBRACE,
    TK_SEMICOLON,

    TK_STAR,
    TK_PLUS,
    TK_MINUS,
    TK_DIV,

    TK_BITWISE_AND,
    TK_BITWISE_OR,
    TK_BITWISE_XOR,

    TK_LOGICAL_AND,
    TK_LOGICAL_OR,

    /** a generated list of tokens */
    TK_SQL_ACCESSIBLE,
    TK_SQL_ACTION,
    TK_SQL_ADD,
    TK_SQL_ALL,
    TK_SQL_ALTER,
    TK_SQL_ANALYZE,
    TK_SQL_AND,
    TK_SQL_AS,
    TK_SQL_ASC,
    TK_SQL_ASENSITIVE,
    TK_SQL_BEFORE,
    TK_SQL_BEGIN,
    TK_SQL_BETWEEN,
    TK_SQL_BIGINT,
    TK_SQL_BINARY,
    TK_SQL_BIT,
    TK_SQL_BLOB,
    TK_SQL_BOTH,
    TK_SQL_BY,
    TK_SQL_CALL,
    TK_SQL_CASCADE,
    TK_SQL_CASE,
    TK_SQL_CHANGE,
    TK_SQL_CHAR,
    TK_SQL_CHARACTER,
    TK_SQL_CHECK,
    TK_SQL_COLLATE,
    TK_SQL_COLUMN,
    TK_SQL_COLUMNS,
    TK_SQL_COMMIT,
    TK_SQL_CONDITION,
    TK_SQL_CONSTRAINT,
    TK_SQL_CONTINUE,
    TK_SQL_CONVERT,
    TK_SQL_CREATE,
    TK_SQL_CROSS,
    TK_SQL_CURRENT_DATE,
    TK_SQL_CURRENT_TIME,
    TK_SQL_CURRENT_TIMESTAMP,
    TK_SQL_CURRENT_USER,
    TK_SQL_CURSOR,
    TK_SQL_DATABASE,
    TK_SQL_DATABASES,
    TK_SQL_DATE,
    TK_SQL_DAY_HOUR,
    TK_SQL_DAY_MICROSECOND,
    TK_SQL_DAY_MINUTE,
    TK_SQL_DAY_SECOND,
    TK_SQL_DEALLOCATE,
    TK_SQL_DEC,
    TK_SQL_DECIMAL,
    TK_SQL_DECLARE,
    TK_SQL_DEFAULT,
    TK_SQL_DELAYED,
    TK_SQL_DELETE,
    TK_SQL_DESC,
    TK_SQL_DESCRIBE,
    TK_SQL_DETERMINISTIC,
    TK_SQL_DISTINCT,
    TK_SQL_DISTINCTROW,
    TK_SQL_DIV,
    TK_SQL_DO,
    TK_SQL_DOUBLE,
    TK_SQL_DROP,
    TK_SQL_DUAL,
    TK_SQL_EACH,
    TK_SQL_ELSE,
    TK_SQL_ELSEIF,
    TK_SQL_ENCLOSED,
    TK_SQL_ENUM,
    TK_SQL_ESCAPED,
    TK_SQL_EXEC,
    TK_SQL_EXECUTE,
    TK_SQL_EXISTS,
    TK_SQL_EXIT,
    TK_SQL_EXPLAIN,
    TK_SQL_END, //add by huih@20151104, for pgbench translation
    TK_SQL_FALSE,
    TK_SQL_FETCH,
    TK_SQL_FLOAT,
    TK_SQL_FLOAT4,
    TK_SQL_FLOAT8,
    TK_SQL_FOR,
    TK_SQL_FORCE,
    TK_SQL_FOREIGN,
    TK_SQL_FROM,
    TK_SQL_FULLTEXT,
    TK_SQL_GLOBAL,
    TK_SQL_GRANT,
    TK_SQL_GROUP,
    TK_SQL_HANDLER,
    TK_SQL_HAVING,
    TK_SQL_HIGH_PRIORITY,
    TK_SQL_HOUR_MICROSECOND,
    TK_SQL_HOUR_MINUTE,
    TK_SQL_HOUR_SECOND,
    TK_SQL_IF,
    TK_SQL_IGNORE,
    TK_SQL_IN,
    TK_SQL_INDEX,
    TK_SQL_INFILE,
    TK_SQL_INNER,
    TK_SQL_INOUT,
    TK_SQL_INSENSITIVE,
    TK_SQL_INSERT,
    TK_SQL_INT,
    TK_SQL_INT1,
    TK_SQL_INT2,
    TK_SQL_INT3,
    TK_SQL_INT4,
    TK_SQL_INT8,
    TK_SQL_INTEGER,
    TK_SQL_INTERVAL,
    TK_SQL_INTO,
    TK_SQL_IS,
    TK_SQL_ITERATE,
    TK_SQL_JOIN,
    TK_SQL_KEY,
    TK_SQL_KEYS,
    TK_SQL_KILL,
    TK_SQL_LEADING,
    TK_SQL_LEAVE,
    TK_SQL_LEFT,
    TK_SQL_LIKE,
    TK_SQL_LIMIT,
    TK_SQL_LINEAR,
    TK_SQL_LINES,
    TK_SQL_LOAD,
    TK_SQL_LOCALTIME,
    TK_SQL_LOCALTIMESTAMP,
    TK_SQL_LOCK,
    TK_SQL_LONG,
    TK_SQL_LONGBLOB,
    TK_SQL_LONGTEXT,
    TK_SQL_LOOP,
    TK_SQL_LOW_PRIORITY,
    TK_SQL_MASTER_SSL_VERIFY_SERVER_CERT,
    TK_SQL_MATCH,
    TK_SQL_MEDIUMBLOB,
    TK_SQL_MEDIUMINT,
    TK_SQL_MEDIUMTEXT,
    TK_SQL_MIDDLEINT,
    TK_SQL_MINUTE_MICROSECOND,
    TK_SQL_MINUTE_SECOND,
    TK_SQL_MOD,
    TK_SQL_MODIFIES,
    TK_SQL_NATURAL,
    TK_SQL_NO,
    TK_SQL_NOT,
    TK_SQL_NO_WRITE_TO_BINLOG,
    TK_SQL_NULL,
    TK_SQL_NUMERIC,
    TK_SQL_ON,
    TK_SQL_OPTIMIZE,
    TK_SQL_OPTION,
    TK_SQL_OPTIONALLY,
    TK_SQL_OR,
    TK_SQL_ORDER,
    TK_SQL_OUT,
    TK_SQL_OUTER,
    TK_SQL_OUTFILE,
	TK_SQL_OFF, //add by huih@20161125 for jdbc
    TK_SQL_PRECISION,
    TK_SQL_PREPARE,
    TK_SQL_PRIMARY,
    TK_SQL_PROCEDURE,
    TK_SQL_PURGE,
    TK_SQL_RANGE,
    TK_SQL_READ,
    TK_SQL_READ_ONLY,
    TK_SQL_READS,
    TK_SQL_READ_WRITE,
    TK_SQL_REAL,
    TK_SQL_REFERENCES,
    TK_SQL_REGEXP,
    TK_SQL_RELEASE,
    TK_SQL_RENAME,
    TK_SQL_REPEAT,
    TK_SQL_REPLACE,
    TK_SQL_REQUIRE,
    TK_SQL_RESTRICT,
    TK_SQL_RETURN,
    TK_SQL_REVOKE,
    TK_SQL_RIGHT,
    TK_SQL_RLIKE,
    TK_SQL_ROLLBACK,
    TK_SQL_SCHEMA,
    TK_SQL_SCHEMAS,
    TK_SQL_SECOND_MICROSECOND,
    TK_SQL_SELECT,
    TK_SQL_SENSITIVE,
    TK_SQL_SEPARATOR,
    TK_SQL_SET,
    TK_SQL_SHOW,
    TK_SQL_SMALLINT,
    TK_SQL_SPATIAL,
    TK_SQL_SPECIFIC,
    TK_SQL_SQL,
    TK_SQL_SQL_BIG_RESULT,
    TK_SQL_SQL_CALC_FOUND_ROWS,
    TK_SQL_SQLEXCEPTION,
    TK_SQL_SQL_SMALL_RESULT,
    TK_SQL_SQLSTATE,
    TK_SQL_SQLWARNING,
    TK_SQL_SSL,
    TK_SQL_START,
    TK_SQL_STARTING,
    TK_SQL_STRAIGHT_JOIN,
    TK_SQL_TABLE,
    TK_SQL_TERMINATED,
    TK_SQL_TEXT,
    TK_SQL_THEN,
    TK_SQL_TIME,
    TK_SQL_TIMESTAMP,
    TK_SQL_TINYBLOB,
    TK_SQL_TINYINT,
    TK_SQL_TINYTEXT,
    TK_SQL_TO,
    TK_SQL_TRAILING,
    TK_SQL_TRANSACTION,
	TK_SQL_TRAN,
    TK_SQL_TRIGGER,
    TK_SQL_TRUE,
    TK_SQL_TRUNCATE,
    TK_SQL_UNDO,
    TK_SQL_UNION,
    TK_SQL_UNIQUE,
    TK_SQL_UNLOCK,
    TK_SQL_UNSIGNED,
    TK_SQL_UPDATE,
    TK_SQL_USAGE,
    TK_SQL_USE,
    TK_SQL_USING,
    TK_SQL_UTC_DATE,
    TK_SQL_UTC_TIME,
    TK_SQL_UTC_TIMESTAMP,
    TK_SQL_VALUE,
    TK_SQL_VALUES,
    TK_SQL_VARBINARY,
    TK_SQL_VARCHAR,
    TK_SQL_VARCHARACTER,
    TK_SQL_VARYING,
    TK_SQL_VACUUM,//add by huih@20151104 for pg
    TK_SQL_WHEN,
    TK_SQL_WHERE,
    TK_SQL_WHILE,
    TK_SQL_WITH,
    TK_SQL_WRITE,
    TK_SQL_X509,
    TK_SQL_XA,
    TK_SQL_XOR,
    TK_SQL_YEAR_MONTH,

    TK_SQL_PARTITION,
    TK_SQL_RETURNING, //add by huih@20151230 for pg
    TK_SQL_ACTIONSCOPE,//:: add by huih@20160118 for pg
    TK_SQL_MATCHREGEXCASE, //add by huih@20160201 for pg ~
    TK_SQL_MATCHREGEXNOCASE, //add by huih@20160201 for pg ~*
    TK_SQL_NOMATCHREGEXCASE, //add by huih@20160201 for pg !~
    TK_SQL_NOMATCHREGEXNOCASE, //add by huih@20160201 for pg !~*
    TK_SQL_DISCARD, //add by huih@20160203 for pg
    TK_SQL_LISTEN, //add by huih@20160203 for pg
    TK_SQL_COMMENT, //add by huih@20160204 for pg
    TK_SQL_NOTIFY, //add by huih@20160204 for pg
    TK_SQL_SAVEPOINT, //add by huih@20160204 for pg
	TK_SQL_IMPLICIT_TRANSACTIONS,//add by huih@20161125 for jdbc.
    TK_SQL_ZEROFILL,

    TK_COMMENT_MYSQL,

    TK_LAST_TOKEN
} sql_token_id;

struct SQLQuery {
    int tbegin;
    int tend;
    int fromstart;
    int fromend;
    int wherestart;
    int whereend;
};

struct SQLTable
{
    int tbegin;
    int tend;
    int fromstart;
    int fromend;
    int wherestart;
    int whereend;
    int big_query;

    bool use_alias_method; //add by huih@20151230
    int replacebgin; //add by huih@20151230, use alias replace table name. the begin of table name
    int replaceend; //add by huih@20151230

    char table[1024];
    char alias[1024];
    int position;

    //GHashTable *keyvalues;
    //GRWLock    rwlock;
};


struct sql_token {
    sql_token_id token_id;
    std::string text;
};

class SqlParser
{
public:
    enum {
        Unknown = 0,
        Select,
        Update,
        Insert,
        Replace,
        Delete,
        Commit,
        Rollback,
    };
    SqlParser(void);
    ~SqlParser(void);

    bool parse(const char* str)
    { return parse(str, strlen(str)); }
    bool parse(const char* str, int len);
    void reset(void);

    int tokenCount(void) const { return m_tokens.size(); }
    sql_token* token(int idx) const { return m_tokens[idx]; }
    void appendToken(sql_token_id id, const char *text, int len = -1);
    void appendTokenToLast(const char *text, int len = -1);

    void toPatternQuery(std::string& s);

    int queryType(void);
    bool isDDLQuery(void);
    bool isDCLQuery(void);

    static const char* tokenNameById(sql_token_id id);
    static sql_token_id tokenIdByName(const char* name, int len);
    static sql_token_id tokenIsLiteral(const char* name, int len);

    int tableCount(void) const { return m_tablelist.size(); }
    const SQLTable* table(int index) const { return &m_tablelist[index]; }

    bool isSelect(){ return this->queryType() == Select;}
    bool isInsert(){ return this->queryType() == Insert;}
    bool isUpdate(){ return this->queryType() == Update;}
    bool isDelete(){ return this->queryType() == Delete;}
    bool isCommit(){ return this->queryType() == Commit;}
    bool isRollBack(){return this->queryType() == Rollback;}
    bool isEndTrans(){return ((this->queryType() == Commit) || (this->queryType() == Rollback));}
    bool isStartTrans();
    void show_tokens(int start = 0, int end = 0);

private:
    bool parseTableNameAndAlias(SQLQuery *query, int start, int end);
    bool parseSimpleTableNameAndAlias(SQLQuery *query, int start, int end);
    bool parseSubQuery(int tbegin, int tend);
    void parseQueryTable(SQLQuery *query);
    bool findToken(int start, int end, int tokId, int *where);
    bool findTokens(int start, int end, int *tokIds, int size, int *where);
    bool getSqlFrom(int begin, int tkend, int *start, int *end);
    bool getSqlWhere(int begin, int tkend, int *start, int *end);
    void handleSubQuery(int substart, int subend);
    void addTableInfo(const std::string& tableName, const std::string& aliasName,
                      int position, int begin, int end, int fromBegin,
                      int fromEnd, int whereBegin, int whereEnd, bool bigquery,
                      bool use_alias_method=false, int replaceBegin = 0, int replaceEnd = 0);
    int skipSomeHeaderTokens(int start, int end, int tokenId);
    int skipExternTokensInHeader(int start, int end);

    sql_token* getSqlTokenFromCache();

public:
    int getQueryType(int ndx);
    int parseToken(const char* str, int len);

private:
    int m_local_begin;
    int m_local_end;
    bool m_isBigQuery;
    bool m_is_insert_select;
    std::vector<SQLTable> m_tablelist;
    std::vector<SQLQuery> m_querylist;
    std::vector<sql_token*> m_tokens;
    std::list<sql_token*> m_tokenCache;
};

#endif


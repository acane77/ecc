#ifndef _MIYUKI_DEBUG_H
#define _MIYUKI_DEBUG_H

// Specify debug level

// ** Deug bug levels **
#define _MIYUKI_DEBUG_LEVEL__SRCMGR  1
#define _MIYUKI_DEBUG_LEVEL__LEXER   2
#define _MIYUKI_DEBUG_LEVEL__PARSER  4
#define _MIYUKI_DEBUG_LEVEL__AST     8
#define _MIYUKI_DEBUG_LEVEL__COMMON  16

#define _MIYUKI_DEBUG_LEVEL_RELEASE 0
#define _MIYUKI_DEBUG_LEVEL_ALL     31

// ** Debug Flag **
#define _MIYUKI_DEBUG_FLAG           _MIYUKI_DEBUG_LEVEL_ALL

#define _MIYUKI_LOG_OUTPUT(msg, title, os) (os << "[" << (title) << "]  " << (msg)) << "\n";
#define _MIYUKI_LOG_STDOUT(msg, title) _MIYUKI_LOG_OUTPUT(msg, title, std::cout)

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__SRCMGR
#define LogSM(title, msg) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogSM(title, msg) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__SRCMGR

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__LEXER
#define LogLexer(title, msg) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogLexer(msg, title) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__LEXER

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__PARSER
#define LogParser(title, msg) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogParser(title, msg)
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__PARSER

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__AST
#define LogAST(title, msg) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogAST(title, msg) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__AST

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__COMMON
#define DebugLog(title, msg) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define DebugLog(title, msg) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__COMMON

#endif
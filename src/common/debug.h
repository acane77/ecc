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

#define _MIYUKI_LOG_OUTPUT(msg, title, os) (os << "[" << (title) << "]  " << (msg))
#define _MIYUKI_LOG_STDOUT(msg, title) _MIYUKI_LOG_OUTPUT(msg, title, std::cout)

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__SRCMGR
#define LogSM(msg, title) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogSM(msg, title) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__SRCMGR

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__LEXER
#define LogLexer(msg, title) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogLexer(msg, title) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__LEXER

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__PARSER
#define LogParser(msg, title) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogParser(msg, title) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__PARSER

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__AST
#define LogAST(msg, title) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define LogAST(msg, title) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__AST

#if _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__COMMON
#define DebugLog(msg, title) _MIYUKI_LOG_STDOUT(msg, title)
#else
#define DebugLog(msg, title) 
#endif // _MIYUKI_DEBUG_FLAG & _MIYUKI_DEBUG_LEVEL__COMMON

#endif
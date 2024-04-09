#pragma once

// std
#include <string>

// parl
#include <analysis/AnalysisVisitor.hpp>
#include <lexer/Lexer.hpp>
#include <parl/Token.hpp>
#include <parser/Parser.hpp>

namespace PArL {

class Runner {
   public:
    Runner(bool dfsaDbg, bool lexerDbg, bool parserDbg);

    int runFile(std::string& path);
    int runPrompt();

    void debugDfsa();
    void debugLexeing(std::string const& source);
    void debugParsing(core::Program* program);

   private:
    void run(std::string const& source);

    bool mHadLexingError = false;
    bool mHadParsingError = false;
    bool mHadAnalysisError = false;

    bool mDfsaDbg = false;
    bool mLexerDbg = false;
    bool mParserDbg = false;

    Lexer mLexer;
    Parser mParser;
    AnalysisVisitor mAnalyser;
};

}  // namespace PArL

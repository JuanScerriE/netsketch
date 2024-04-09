// std
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

// parl
#include <ir_gen/GenVisitor.hpp>
#include <lexer/LexerDirector.hpp>
#include <parl/Token.hpp>
#include <parser/Parser.hpp>
#include <parser/PrinterVisitor.hpp>
#include <preprocess/ReorderVisitor.hpp>
#include <runner/Runner.hpp>

// fmt
#include <fmt/core.h>

namespace PArL {

Runner::Runner(bool dfsaDbg, bool lexerDbg, bool parserDbg)
    : mDfsaDbg(dfsaDbg),
      mLexerDbg(lexerDbg),
      mParserDbg(parserDbg),
      mLexer(LexerDirector::buildLexer()),
      mParser(Parser(mLexer)) {
}

// static inline size_t intStringLen(size_t integer) {
//     size_t length = 1;
//
//     while (0 != (integer /= 10))
//         length++;
//
//     return length;
// }

// void Runner::debugDfsa() {
//     fmt::println("Dfsa Debug Print");
//
//     Dfsa dfsa = mLexer.getDfsa();
//
//     // we use +1 to handle the fact that we will most
//     // likely have a '-' in front and that's an extra
//     // character.
//     size_t length = intStringLen(dfsa.mNoOfStates) + 1;
//
//     fmt::println("Accepting States:\n\t{}",
//                  fmt::join(mFinalStates, ","));
//
//     fmt::println("Initial State:\n\t{}", mInitialState);
//
//     fmt::println("Transition Table:");
//
//     for (int i = 0; i < mNoOfStates; i++) {
//         for (int j = -1; j < mNoOfCategories; j++) {
//             if (j == -1) {
//                 fmt::print("\t{0: {1}}: ", i, length);
//             } else {
//                 fmt::print("{0: {1}}",
//                            mTransitionTable[i][j],
//                            length);
//
//                 if (j < mNoOfCategories - 1)
//                     fmt::print(", ");
//             }
//         }
//
//         fmt::print("\n");
//     }
// }
// mLexer.getDfsa().print();
//
// fmt::print("\n");
// }

void Runner::debugLexeing(std::string const& source) {
    fmt::println("Lexer Debug Print");

    mLexer.addSource(source);

    for (;;) {
        std::optional<Token> token = mLexer.nextToken();

        if (token.has_value()) {
            fmt::println(
                "{}:{} {}",
                token->getPosition().row(),
                token->getPosition().col(),
                token->toString()
            );

            if (token->getType() ==
                Token::Type::END_OF_FILE) {
                break;
            }
        }
    }

    mLexer.reset();
}

void Runner::debugParsing(core::Program* program) {
    fmt::println("Parser Debug Print");

    PrinterVisitor printer;

    program->accept(&printer);
}

void Runner::run(std::string const& source) {
    if (mLexerDbg) {
        debugLexeing(source);
    }

    mParser.parse(source);

    if (mLexer.hasError() || mParser.hasError()) {
        return;
    }

    std::unique_ptr<core::Program> ast = mParser.getAst();

    if (mParserDbg) {
        debugParsing(ast.get());
    }

    mAnalyser.analyse(ast.get());

    if (mAnalyser.hasError()) {
        return;
    }

    std::unique_ptr<Environment> environment =
        mAnalyser.getEnvironment();

    ReorderVisitor reorder{};

    reorder.reorderAst(ast.get());

    if (mParserDbg) {
        debugParsing(ast.get());
    }

    reorder.reorderEnvironment(environment.get());

    GenVisitor gen{environment.get()};

    ast->accept(&gen);

    gen.print();
}

int Runner::runFile(std::string& path) {
    std::filesystem::path fsPath{path};

    if (!std::filesystem::exists(fsPath)) {
        fmt::println(stderr, "parl: path does not exist");

        return EXIT_FAILURE;
    }

    if (!std::filesystem::is_regular_file(fsPath)) {
        fmt::println(
            stderr,
            "parl: path is not a regular file"
        );

        return EXIT_FAILURE;
    }

    std::ifstream file(path);

    // make sure the file is opened correctly
    if (!file) {
        fmt::println(stderr, "parl: {}", strerror(errno));

        return EXIT_FAILURE;
    }

    // get the length of the file
    file.seekg(0, std::ifstream::end);
    auto length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    // load the source file into a string
    std::string source(length, '\0');
    file.read(source.data(), length);

    // close file
    file.close();

    // run the source file
    run(source);

    if (mHadLexingError || mHadParsingError)
        return 65;

    return 0;
}

int Runner::runPrompt() {
    std::string line;

    for (;;) {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line.empty()) {
            break;
        }

        run(line);

        mAnalyser.reset();

        mHadLexingError = false;
        mHadParsingError = false;
        mHadAnalysisError = false;
    }

    return 0;
}

}  // namespace PArL

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cmath>

enum class TokenType {
    Number,
    Plus,
    Minus,
    Multiply,
    Divide,
    LeftParen,
    RightParen,
    Identifier,
    Assignment,
    Comma,
    Function,
    End
};

struct Token {
    TokenType type;
    std::string text;
};

class Lexer {
public:
    Lexer(const std::string& input) : input(input), pos(0) {}
    Token getNextToken();

private:
    std::string input;
    size_t pos;
    char currentChar() const { return input[pos]; }
    void advance() { pos++; }
    void skipWhitespace();
    Token number();
    Token identifier();
};

void Lexer::skipWhitespace() {
    while (isspace(currentChar())) {
        advance();
    }
}

Token Lexer::number() {
    std::string value;
    while (isdigit(currentChar())) {
        value += currentChar();
        advance();
    }
    return { TokenType::Number, value };
}

Token Lexer::identifier() {
    std::string value;
    while (isalnum(currentChar())) {
        value += currentChar();
        advance();
    }
    if (value == "pow" || value == "abs" || value == "max" || value == "min") {
        return { TokenType::Function, value };
    }
    return { TokenType::Identifier, value };
}

Token Lexer::getNextToken() {
    while (currentChar() != '\0') {
        if (isspace(currentChar())) {
            skipWhitespace();
            continue;
        }
        if (isdigit(currentChar())) {
            return number();
        }
        if (isalnum(currentChar())) {
            return identifier();
        }
        switch (currentChar()) {
            case '+': advance(); return { TokenType::Plus, "+" };
            case '-': advance(); return { TokenType::Minus, "-" };
            case '*': advance(); return { TokenType::Multiply, "*" };
            case '/': advance(); return { TokenType::Divide, "/" };
            case '(': advance(); return { TokenType::LeftParen, "(" };
            case ')': advance(); return { TokenType::RightParen, ")" };
            case '=': advance(); return { TokenType::Assignment, "=" };
            case ',': advance(); return { TokenType::Comma, "," };
            default: throw std::runtime_error("Invalid character");
        }
    }
    return { TokenType::End, "" };
}

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual double evaluate() const = 0;
};

class NumberNode : public ASTNode {
public:
    explicit NumberNode(double value) : value(value) {}
    double evaluate() const override { return value; }

private:
    double value;
};

class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(char op, ASTNode* left, ASTNode* right)
            : op(op), left(left), right(right) {}
    ~BinaryOpNode() {
        delete left;
        delete right;
    }
    double evaluate() const override;

private:
    char op;
    ASTNode* left;
    ASTNode* right;
};

double BinaryOpNode::evaluate() const {
    double leftValue = left->evaluate();
    double rightValue = right->evaluate();
    switch (op) {
        case '+': return leftValue + rightValue;
        case '-': return leftValue - rightValue;
        case '*': return leftValue * rightValue;
        case '/': return leftValue / rightValue;
        default: throw std::runtime_error("Invalid binary operator");
    }
}

class VariableNode : public ASTNode {
public:
    explicit VariableNode(const std::string& name) : name(name) {}
    double evaluate() const override;

private:
    std::string name;
};

double VariableNode::evaluate() const {
    throw std::runtime_error("Variable evaluation not implemented");
}

class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode(const std::string& name, std::vector<ASTNode*> args)
            : name(name), args(std::move(args)) {}
    ~FunctionCallNode() {
        for (ASTNode* arg : args) {
            delete arg;
        }
    }
    double evaluate() const override;

private:
    std::string name;
    std::vector<ASTNode*> args;
};

double FunctionCallNode::evaluate() const {
    throw std::runtime_error("Function call evaluation not implemented");
}

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}
    ASTNode* parse();

private:
    std::vector<Token> tokens;
    size_t pos;
    Token currentToken() const { return tokens[pos]; }
    void advance() { pos++; }
    ASTNode* expression();
    ASTNode* term();
    ASTNode* factor();
    ASTNode* primary();
};

ASTNode* Parser::parse() {
    return expression();
}

ASTNode* Parser::expression() {
    ASTNode* node = term();
    while (currentToken().type == TokenType::Plus || currentToken().type == TokenType::Minus) {
        char op = currentToken().text[0];
        advance();
        node = new BinaryOpNode(op, node, term());
    }
    return node;
}

ASTNode* Parser::term() {
    ASTNode* node = factor();
    while (currentToken().type == TokenType::Multiply || currentToken().type == TokenType::Divide) {
        char op = currentToken().text[0];
        advance();
        node = new BinaryOpNode(op, node, factor());
    }
    return node;
}

ASTNode* Parser::factor() {
    ASTNode* node = primary();
    return node;
}

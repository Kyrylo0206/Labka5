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
ASTNode* Parser::primary() {
    if (currentToken().type == TokenType::Number) {
        double value = std::stod(currentToken().text);
        advance();
        return new NumberNode(value);
    }
    else if (currentToken().type == TokenType::LeftParen) {
        advance();
        ASTNode* node = expression();
        if (currentToken().type != TokenType::RightParen) {
            throw std::runtime_error("Mismatched parentheses");
        }
        advance();
        return node;
    }
    else if (currentToken().type == TokenType::Identifier) {
        std::string name = currentToken().text;
        advance();
        return new VariableNode(name);
    }
    else if (currentToken().type == TokenType::Function) {
        std::string name = currentToken().text;
        advance();
        if (currentToken().type != TokenType::LeftParen) {
            throw std::runtime_error("Expected '(' after function name");
        }
        advance();
        std::vector<ASTNode*> args;
        while (currentToken().type != TokenType::RightParen) {
            args.push_back(expression());
            if (currentToken().type == TokenType::Comma) {
                advance();
            }
        }
        advance();
        return new FunctionCallNode(name, args);
    }
    throw std::runtime_error("Invalid primary expression");
}

class Interpreter {
public:
    double evaluate(const std::string& input);

private:
    std::map<std::string, double> variables;
    std::map<std::string, std::function<double(double, double)>> functions = {
            {"pow", [](double a, double b) { return std::pow(a, b); }},
            {"abs", [](double a, double) { return std::abs(a); }},
            {"max", [](double a, double b) { return std::max(a, b); }},
            {"min", [](double a, double b) { return std::min(a, b); }}
    };
};

double Interpreter::evaluate(const std::string& input) {
    Lexer lexer(input);
    std::vector<Token> tokens;
    Token token = lexer.getNextToken();
    while (token.type != TokenType::End) {
        tokens.push_back(token);
        token = lexer.getNextToken();
    }
    Parser parser(tokens);
    ASTNode* root = parser.parse();
    double result = root->evaluate();
    delete root;
    return result;
}

int main() {
    std::string input;
    std::cout << "> ";
    std::getline(std::cin, input);

    try {
        Interpreter interpreter;
        double result = interpreter.evaluate(input);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

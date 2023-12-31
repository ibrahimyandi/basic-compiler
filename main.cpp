#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

enum TokenType {
    END,        // ""
    COMMA,      // ,
    NUMBER,     // [0-9]
    PLUS,       // +
    PRINT,      // yazdir
    RANGE,      // sayiDizisi
    SUBSTRING,  // altDize
    LEFTPAREN,  // (
    RIGHTPAREN, // )
    STRING,     // "
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
    std::string input;
    size_t position;

public:
    Lexer(const std::string& input) : input(input), position(0) {}

    Token getNextToken() {
        if (position >= input.length()) {
            return {END, ""};
        } 
        else if (input[position] == ',')
        {
            ++position;
            return {COMMA, ","};
        } 
        else if (std::isdigit(input[position])) {
            return {NUMBER, parseNumber()};
        } 
        else if (input[position] == '+') {
            ++position;
            return {PLUS, "+"};
        }
        else if (input.substr(position, 6) == "yazdir" && !isalnum(input[position + 6]))
        {
            position += 6;
            return {PRINT, "yazdir"};
        }
        else if (input.substr(position, 10) == "sayiDizisi")
        {
            position += 10;
            return {RANGE, "sayiDizisi"};
        }
        else if (input.substr(position, 7) == "altDize")
        {
            position += 7;
            return {SUBSTRING, "altDize"};
        }
        else if (input[position] == '(')
        {
            ++position;
            return {LEFTPAREN, "("};
        }
        else if (input[position] == ')')
        {
            ++position;
            return {RIGHTPAREN, ")"};
        }
        else if (input[position] == '"')
        {
            ++position;
            return {STRING, parseString()};
        }

        throw std::runtime_error("Bilinmeyen karakter");
    }

private:
    std::string parseNumber() {
        std::stringstream number;
        while (position < input.length() && std::isdigit(input[position])) {
            number << input[position++];
        }

        return number.str();
    }

    std::string parseString() {
        std::stringstream str;
        while (position < input.length() && input[position] != '"') {
            str << input[position++];
        }
        if (position < input.length() && input[position] == '"') {
            ++position;
        }

        return str.str();
    }
};

class Parser {
    Lexer& lexer;
    Token currentToken;

public:
    Parser(Lexer& lexer) : lexer(lexer) {
        currentToken = lexer.getNextToken();
    }

    int parse() {
        int result = parseExpression();
        if (currentToken.type != END) {
            std::cout << "Beklenmeyen token: " << currentToken.value << std::endl;
            throw std::runtime_error("Beklenmeyen token: " + currentToken.value);
        }
        return result;
    }

private:
    int parseExpression() {
        int result = parseTerm();
        while (currentToken.type == PLUS) {
            Token op = currentToken;
            currentToken = lexer.getNextToken();
            int right = parseTerm();
            if (op.type == PLUS) {
                result += right;
            }
        }
        return result;
    }

    int parseTerm() {
        Token token = currentToken;

        if (token.type == NUMBER) {
            currentToken = lexer.getNextToken();
            return std::stoi(token.value);
        }
        else if (token.type == STRING) {
            currentToken = lexer.getNextToken();
            return token.value.length();
        }
        else if (token.type == PRINT) {
            currentToken = lexer.getNextToken();
            match(LEFTPAREN);
            if (currentToken.type == STRING) {
                std::cout << parseString() << std::endl;
            }
            else if (currentToken.type == RANGE)
            {
                currentToken = lexer.getNextToken();
                match(LEFTPAREN);
                int end = parseExpression();
                int start = 0;
                int step = 1;

                if (currentToken.type == COMMA) {
                    match(COMMA);
                    start = end;
                    end = parseExpression();
                }

                if (currentToken.type == COMMA)
                {
                    match(COMMA);
                    step = parseExpression();
                }
                
                match(RIGHTPAREN);

                for (int i = start; i < end; i += step) {
                    std::cout << i << " ";
                }
                std::cout << std::endl;
            }
            else if (currentToken.type == SUBSTRING)
            {
                currentToken = lexer.getNextToken();
                match(LEFTPAREN);
                std::string text = "";
                int startIndex = 0;
                int length;
                if (currentToken.type == STRING) {
                    text = parseString();
                    length = text.length();
                }
                if (currentToken.type == COMMA) {
                    match(COMMA);
                    startIndex = parseExpression();
                }
                if (currentToken.type == COMMA) {
                    match(COMMA);
                    length = parseExpression() + startIndex;
                }
                match(RIGHTPAREN);

                for (size_t i = startIndex; i < length; i++)
                {
                    std::cout << text[i];
                }
            }
            
            else {
                std::cout << parseExpression() << std::endl;
            }
            match(RIGHTPAREN);
            return 0;
        }
        else if (token.type == LEFTPAREN) {
            match(LEFTPAREN);
            int result = parseExpression();
            match(RIGHTPAREN);
            return result;
        }
        else {
            throw std::runtime_error("Beklenmeyen token: " + token.value);
        }
    }

    void match(TokenType expectedType) {
        if (currentToken.type == expectedType) {
            currentToken = lexer.getNextToken();
        } else {
            std::cerr << "Beklenen token: " << expectedType << ", Alinan token: " << currentToken.type << std::endl;
            throw std::runtime_error("Beklenmeyen token");
        }
    }

    std::string parseString() {
        Token token = currentToken;
        match(STRING);
        return token.value;
    }
};

void CompileLine(const std::string& line) {
    Lexer lexer(line);
    Parser parser(lexer);

    try {
        int result = parser.parse();
    } catch (const std::exception& e) {
        std::cerr << "Hata: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Kullanim: " << argv[0] << " <metin>" << std::endl;
        return 1;
    }
    std::string text = argv[1];
    std::ifstream file(text);

    std::string line;

    while (std::getline(file, line)) {
        CompileLine(line);
    }

    return 0;
}
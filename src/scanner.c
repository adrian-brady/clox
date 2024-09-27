#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct scanner {
  const char* start;
  const char* current;
  int line;
} Scanner;

Scanner scanner;

/**
 * Initializes the values of the scanner using the input string provided.
 *
 * @param source The input to scan with the scanner.
 *
 * @side_effects
 * - scanner.start is set to 'source'
 * - scanner.current is set to 'source'
 * - scanner.line is set to 1
 */
void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static bool isAlpha(char c) {
  return  (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          (c == '_');
}

/**
 * @brief Check if the given character is a number.
 *
 * This function checks whether the given character is a decimal number between 0 and 9.
 * 
 * @return [bool] Returns 'true' if the given character is a number between 0 and 9, and
 * returns 'false' if the character is not a number between 0 and 9.
 *
 * @side_effects
 * - None
 */
static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

/**
 * @brief Check if the scanner has reached the end of its input.
 *
 * This static function checks whether the current character pointed to by the scanner
 * is the null terminator (`'\0'`), indicating the end of the input string.
 *
 * @return [bool] Returns `true` if the current character is the null terminator (`'\0'`),
 * indicating the end of the input; otherwise, returns `false`.
 *
 * @side_effects
 * - None.
 */
static bool isAtEnd() {
  return *scanner.current == '\0';
}

/**
 * @brief Advance the character pointer of the scanner.
 *
 * This static function increments the `current` field of the scanner, returning the
 * previously pointed to character.
 *
 * @return [char] Returns the char previously pointed to by the scanner.
 *
 * @side_effects
 * - Increments `scanner.current`.
 */
static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

/**
 * @brief Return the scanner's current char.
 *
 * This function returns the char pointed to by the scanner's 'current' field.
 *
 * @return [char] The char pointed to by the scanner's 'current' field.
 *
 * @side_effects
 * - None
 */
static char peek() {
  return *scanner.current;
}

/**
 * @brief Return the scanner's next char, if the next char is not the end of the input.
 *
 * This function returns the character next of the current character pointed to by the
 * scanner's 'current' field.
 *
 * @return [char] The char next of the current char pointed to by the 'current' field.
 *
 * @side_effects
 * - None
 */
static char peekNext() {
  if (isAtEnd()) return '\0';
  return scanner.current[1];
}

/**
 * @brief Conditionally consume the next character.
 *
 * This function conditionally consumes the next character of the scanner if the given
 * char and the scanner's current char are equal.
 *
 * @param
 * - expected [char] The char to check equality of.
 *
 * @return [bool] Returns `true` if the given character and the character pointed
 * to by the scanner are equal. Returns `false` if the scanner reached the end of its
 * input or if the given char and the char pointed to by the scanner are not equal.
 *
 * @side_effects
 * - Increments 'scanner.current' if returning `true`.
 */
static bool match(char expected) {
  if (isAtEnd()) return false;
  if (*scanner.current != expected) return false;
  scanner.current++;
  return true;
}

/**
 * @brief Creates a new token utilizing the state of the scanner.
 *
 * This function creates a new token. The token's `start`, `length`, and `line` fields
 * are determined by the state of the scanner. The `type` is given by a parameter.
 *
 * @param
 * - type [TokenType] The type of the token
 *
 * @return [token] Returns a newly created token.
 *
 * @side_effects
 * - None
 */
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

/**
 * @brief Creates a new error token for error reporting.
 *
 * This function creates a new error token. It utilizes the token struct's start field
 * to pass around an error message.
 *
 * @param
 * - message [char*] The error message for the token to hold.
 *
 * @return [token] Returns a new error token.
 *
 * @side_effects
 * - None
 */
static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

static void skipWhitespace() {
  for (;;) {
    char c = (char)peek();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
      case '\n':
        scanner.line++;
        advance();
        break;
      case '/':
        if (peekNext() == '/') {
          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static TokenType checkKeyword(int start, int length,
      const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (scanner.start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
        }
      }
    break;
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
    break;
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }

  return TOKEN_IDENTIFIER;
}

static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek())) advance();
  return makeToken(identifierType());
}


static Token number() {
  while (isDigit(peek())) advance();

  // Look for fractional part of number
  if (peek() == '.' && isDigit(peekNext())) {
    // consume '.'
    advance();

    while (isDigit(peek())) advance();
  }

  return makeToken(TOKEN_NUMBER);
}

static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') scanner.line++;
    advance();
  }

  if (isAtEnd()) return errorToken("Unterminated string.");

  // Consume closing quote.
  advance();
  return makeToken(TOKEN_STRING);
}

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;
  if (isAtEnd()) return makeToken(TOKEN_EOF);

  char c = advance();
  if (isAlpha(c)) return identifier();
  if (isDigit(c)) return number();

  switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '-': return makeToken(TOKEN_MINUS);
    case '+': return makeToken(TOKEN_PLUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '*': return makeToken(TOKEN_STAR);
    case '!':
      return makeToken(
        match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(
        match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(
        match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(
        match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return string();
  }

  return errorToken("Unexpected character.");
}

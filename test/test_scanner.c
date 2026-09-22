#include "unity.h"
#include "parser.tab.h" 

TEST_SOURCE_FILE("lex.yy.c")

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yylex(void);
extern char *yytext;

void setUp(void) {
}

void tearDown(void) {
    // Se ejecuta después de cada test.
}

static void probar_token(const char* input, int expected_token) {
    YY_BUFFER_STATE buffer = yy_scan_string(input);
    int actual_token = yylex();
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected_token, actual_token, "El token devuelto no es el esperado");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(input, yytext, "El texto consumido (yytext) no coincide");
    yy_delete_buffer(buffer);
}

void test_data_type_scanner(void) {
    probar_token("int", TYPE);
    probar_token("boolean", TYPE);
    probar_token("float", TYPE);
    probar_token("void", VOID);
}

void test_reserved_words_scanner(void) {
    probar_token("if", IF);
    probar_token("else", ELSE);
    probar_token("while", WHILE);
    probar_token("return", RETURN);
}

void test_boolean_scanner(void) {
    probar_token("true", BOOL);
    probar_token("false", BOOL);
}

void test_pattern_scanner(void) {
    probar_token("a", ALPHA);
    probar_token("Z", ALPHA);
    probar_token("5", DIGIT);
}

void test_operator_scanner(void) {
    probar_token("&&", AND_OP);
    probar_token("||", OR_OP);
    probar_token("!", EXCLAMATION);
    probar_token("+", ADD_OP);
    probar_token("-", SUBTRACT_OP);
    probar_token("*", MULT_OP);
    probar_token("/", DIV_OP);
    probar_token("%", MOD_OP);
    probar_token("=", ASIGN_OP);
    probar_token("<", LESS_OP);
    probar_token(">", GREATER_OP);
}

void test_gruping_scanner(void) {
    probar_token(".", DOT);
    probar_token(",", COMMA);
    probar_token(";", SEMICOLON);
    probar_token("(", LEFT_PARENTHESIS);
    probar_token(")", RIGHT_PARENTHESIS);
    probar_token("{", LEFT_BRACE);
    probar_token("}", RIGHT_BRACE);
    probar_token("_", UNDERSCORE);
}

void test_ignored_tokens_scanner(void) {
    probar_token("//", COMENT); 
    probar_token("\n", NEW_LINE);
    probar_token("\t", TAB);
    probar_token(" ", SPACE);
}
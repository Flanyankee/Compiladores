#include "unity.h"
#include "parser.tab.h" 
#include "lex.yy.c"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yyparse(void);

void setUp(void) {}

void tearDown(void) {}

static int parse_string(const char* input) {
    YY_BUFFER_STATE buffer = yy_scan_string(input);
    int result = yyparse(); 
    yy_delete_buffer(buffer);
    return result;
}

void test_parser_declaration_of_var(void) {
    const char* code = "int myVar_a;";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser has rejected a valid variable declaration");
}

void test_parser_invalid_declaration_of_var(void) {
    const char* code = "int myVar"; 
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser accepted code with invalid syntax");
}

void test_parser_valid_int_method_declare(void){
    const char* code = "int main(){}"; 
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid method declaration");
}

void test_parser_invalid_int_method_declare(void){
    const char* code = "int main();{"; 
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid method declaration");
}

void test_parser_valid_void_method_declare(void){
    const char* code = "void main(){}"; 
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid method declaration");
}

void test_parser_invalid_void_method_declare(void){
    const char* code = "void main();{"; 
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid method declaration");
}

void test_parser_valid_method_call(void){
    const char* code = "int test_method() { main(x); }"; 
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "El parser rechazó una llamada de método válida");
}

void test_parser_invalid_method_call(void){
    const char* code = "int test_method() { main(x}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid method call");
}

void test_parser_valid_asing_operation(void){
    const char* code = "int main(){x = 2;}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid assignment");
}

void test_parser_invalid_asing_operation(void){
    const char* code = "int main(){x = 2}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid assignment");
}

void test_parser_valid_if_then_operation(void){
    const char* code = "int main(){if(!(x == 5)){}}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid ‘if then’ operation");
}

void test_parser_invalid_if_then_operation(void){
    const char* code = "int main(){if(a < 3 < p)){}}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid ‘if then’ operation");
}

void test_parser_valid_if_then_else_operation(void){
    const char* code = "int main(){if(x + (-4) == 4){} else{int x;} }";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid ‘if-then-else’ operation");
}

void test_parser_invalid_if_then_else_operation(void){
    const char* code = "int main(){if(a || b)){}else{x}}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid if-then-else statement");
}

void test_parser_valid_return_operation(void){
    const char* code = "void main(){return;}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid return operation");
}

void test_parser_invalid_return_operation(void){
    const char* code = "void main(){return}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid return operation");
}

void test_parser_valid_return_expr_operation(void){
    const char* code = "int main(){return -x;}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid ‘return expression’ operation");
}

void test_parser_invalid_return_expr_operation(void){
    const char* code = "int main(){return x}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid ‘return expression’ operation");
}

void test_parser_valid_while_operation(void){
    const char* code = "int main(){while(true){x = (x+1);}}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "The parser rejected a valid ‘while’ operation");
}

void test_parser_invalid_while_operation(void){
    const char* code = "int main(){while(){x = x+1}}";
    int result = parse_string(code);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result, "The parser encountered an invalid `while` statement");
}
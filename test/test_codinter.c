#include "unity.h"
#include "parser.tab.h"
#include "scanner.h"
#include "syntax-tree.h"
#include "symbols-tab.h"
#include "semantic-analizer.h"
#include "codinter.h"

TEST_SOURCE_FILE("lex.yy.c")
TEST_SOURCE_FILE("parser.tab.c")
TEST_SOURCE_FILE("syntax-tree.c")
TEST_SOURCE_FILE("symbols-tab.c")
TEST_SOURCE_FILE("semantic-analizer.c")
TEST_SOURCE_FILE("codinter.c")

FILE *outputFilePtr = NULL;

void setUp(void) {
    outputFilePtr = stdout;
    semanticOutputFile = stdout;
    initCodinterGenerator();
}

void tearDown(void) {}

void test_generacion_codinter_if_else_y_aritmetica(void) {
    const char *codigo =
        "int main() {\n"
        "    int x;\n"
        "    boolean y;\n"
        "    y = true;\n"
        "    x = 10;\n"
        "    if (x > 5) {\n"
        "        x = x + 1;\n"
        "    } else {\n"
        "        x = 0;\n"
        "    }\n"
        "    return x;\n"
        "}\n";

    YY_BUFFER_STATE buffer = yy_scan_string(codigo);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, yyparse(), "parse error");

    SymbolTab *st = initializeSymbolTab();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, analyzeAst(syntaxTree, st), "Semantic analizer error");

    generateCodinterFromAST(syntaxTree);

    CodinterInstruction *head = getCodinterListHead();
    TEST_ASSERT_NOT_NULL_MESSAGE(head, "The Codinter instruction list shuld not bee NULL");
    TEST_ASSERT_EQUAL_INT(Codinter_FUNC_BEGIN, head->op);
    TEST_ASSERT_EQUAL_STRING("main", head->arg1->id);

    printf("\n The intermediate Code: \n");
    printCodinter(stdout);
    printf("----------------------------------\n");

    yy_delete_buffer(buffer);
}
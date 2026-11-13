#ifdef TEST

#include "unity.h"
#include "parser.tab.h"
#include "scanner.h"
#include "constants.h"
#include "syntax-tree.h"
#include "symbols-tab.h"
#include "semantic-analizer.h"
#include "optimizations.h"

TEST_SOURCE_FILE("lex.yy.c")
TEST_SOURCE_FILE("parser.tab.c")
TEST_SOURCE_FILE("syntax-tree.c")
TEST_SOURCE_FILE("symbols-tab.c")
TEST_SOURCE_FILE("semantic-analizer.c")

FILE *outputFilePtr = NULL;

/* Helpers para preparar el AST y consultar nodos */
static void parseAndAnalyze(const char *codigo) {
    YY_BUFFER_STATE buffer = yy_scan_string(codigo);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, yyparse(), "Parse error");

    SymbolTab *st = initializeSymbolTab();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, analyzeAst(syntaxTree, st), "Semantic analyzer error");

    yy_delete_buffer(buffer);
}

static ASTNode *findNodeByType(ASTNode *node, enum symbolTypes targetType) {
    if (node == NULL) {
        return NULL;
    }
    if (node->symbolData != NULL && node->symbolData->symbolType == targetType) {
        return node;
    }
    ASTNode *leftMatch = findNodeByType(node->left, targetType);
    if (leftMatch != NULL) {
        return leftMatch;
    }
    return findNodeByType(node->right, targetType);
}

static ASTNode *findIntLiteral(ASTNode *node, int expectedValue) {
    if (node == NULL) {
        return NULL;
    }
    if (node->symbolData != NULL &&
        node->symbolData->symbolType == intLiteral &&
        node->symbolData->value.intValue == expectedValue) {
        return node;
    }
    ASTNode *leftMatch = findIntLiteral(node->left, expectedValue);
    if (leftMatch != NULL) {
        return leftMatch;
    }
    return findIntLiteral(node->right, expectedValue);
}

static ASTNode *findFloatLiteral(ASTNode *node, float expectedValue, float tolerance) {
    if (node == NULL) {
        return NULL;
    }
    if (node->symbolData != NULL && node->symbolData->symbolType == floatLiteral) {
        float diff = node->symbolData->value.floatValue - expectedValue;
        if (diff < 0.0f) {
            diff = -diff;
        }
        if (diff <= tolerance) {
            return node;
        }
    }
    ASTNode *leftMatch = findFloatLiteral(node->left, expectedValue, tolerance);
    if (leftMatch != NULL) {
        return leftMatch;
    }
    return findFloatLiteral(node->right, expectedValue, tolerance);
}

void setUp(void) {
    outputFilePtr = stdout;
    semanticOutputFile = stdout;
    syntaxTree = NULL;
}

void tearDown(void) {
    if (syntaxTree != NULL) {
        freeAST(syntaxTree);
        syntaxTree = NULL;
    }
}

void test_constant_folding_aritmetica_enteros(void) {
    const char *codigo =
        "int main() {\n"
        "    int x;\n"
        "    x = (2 + 3) * 4 - 6 / 2;\n"
        "    return x;\n"
        "}\n";

    parseAndAnalyze(codigo);
    constantOptimizations(constants);

    TEST_ASSERT_NOT_NULL(syntaxTree);
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, addOp), "addOp deberia haberse plegado");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, multOp), "multOp deberia haberse plegado");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, divOp), "divOp deberia haberse plegado");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, substractOp), "substractOp deberia haberse plegado");

    ASTNode *folded = findIntLiteral(syntaxTree, 17);
    TEST_ASSERT_NOT_NULL_MESSAGE(folded, "La expresion (2 + 3) * 4 - 6 / 2 debe plegarse al literal 17");
    TEST_ASSERT_EQUAL_INT(1, folded->symbolData->hasValue);
}

void test_constant_folding_booleanos_y_negacion(void) {
    const char *codigo =
        "int main() {\n"
        "    boolean b;\n"
        "    b = !(true && false) || false;\n"
        "    return 0;\n"
        "}\n";

    parseAndAnalyze(codigo);
    constantOptimizations(constants);

    TEST_ASSERT_NOT_NULL(syntaxTree);
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, andOp), "andOp deberia haberse plegado");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, negationOp), "negationOp deberia haberse plegado");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, orOp), "orOp deberia haberse plegado");

    ASTNode *boolRes = findNodeByType(syntaxTree, boolLiteral);
    TEST_ASSERT_NOT_NULL_MESSAGE(boolRes, "Debe quedar un boolLiteral resultado del plegado");
    TEST_ASSERT_EQUAL_INT(1, boolRes->symbolData->value.boolValue);
}

void test_constant_folding_parcial_con_variables(void) {
    const char *codigo =
        "int main() {\n"
        "    int x;\n"
        "    x = 10;\n"
        "    x = x + (3 * 4);\n"
        "    return x;\n"
        "}\n";

    parseAndAnalyze(codigo);
    constantOptimizations(constants);

    TEST_ASSERT_NOT_NULL(syntaxTree);
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, multOp), "3 * 4 debe plegarse a 12");
    TEST_ASSERT_NOT_NULL_MESSAGE(findNodeByType(syntaxTree, addOp), "x + 12 no puede plegarse porque x es variable");
    TEST_ASSERT_NOT_NULL_MESSAGE(findIntLiteral(syntaxTree, 12), "El literal 12 debe estar en el AST");
}

void test_dead_code_eliminacion_despues_de_return(void) {
    const char *codigo =
        "int main() {\n"
        "    int x;\n"
        "    x = 10;\n"
        "    return x;\n"
        "    x = 999;\n"
        "    while (true) {\n"
        "        x = x + 1;\n"
        "    }\n"
        "}\n";

    parseAndAnalyze(codigo);
    deadCodeOptimizations(dead_code);

    TEST_ASSERT_NOT_NULL(syntaxTree);
    TEST_ASSERT_NOT_NULL_MESSAGE(findNodeByType(syntaxTree, returnNode), "El return principal debe conservarse");
    TEST_ASSERT_NULL_MESSAGE(findIntLiteral(syntaxTree, 999), "La asignacion posterior al return debe eliminarse");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, whileNode), "El while posterior al return debe eliminarse");
}

void test_dead_code_eliminacion_if_false_y_while_false(void) {
    const char *codigo =
        "int main() {\n"
        "    int x;\n"
        "    x = 5;\n"
        "    if (false) {\n"
        "        x = 100;\n"
        "    }\n"
        "    while (false) {\n"
        "        x = 200;\n"
        "    }\n"
        "    return x;\n"
        "}\n";

    parseAndAnalyze(codigo);
    deadCodeOptimizations(dead_code);

    TEST_ASSERT_NOT_NULL(syntaxTree);
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, ifNode), "if (false) debe eliminarse");
    TEST_ASSERT_NULL_MESSAGE(findNodeByType(syntaxTree, whileNode), "while (false) debe eliminarse");
    TEST_ASSERT_NULL_MESSAGE(findIntLiteral(syntaxTree, 100), "El cuerpo del if (false) debe haberse liberado");
    TEST_ASSERT_NULL_MESSAGE(findIntLiteral(syntaxTree, 200), "El cuerpo del while (false) debe haberse liberado");
}

#endif
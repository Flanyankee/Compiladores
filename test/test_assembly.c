#include "unity.h"
#include "parser.tab.h"
#include "scanner.h"
#include "syntax-tree.h"
#include "symbols-tab.h"
#include "semantic-analizer.h"
#include "codinter.h"
#include "assembly.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

TEST_SOURCE_FILE("lex.yy.c")
TEST_SOURCE_FILE("parser.tab.c")
TEST_SOURCE_FILE("syntax-tree.c")
TEST_SOURCE_FILE("symbols-tab.c")
TEST_SOURCE_FILE("semantic-analizer.c")
TEST_SOURCE_FILE("codinter.c")
TEST_SOURCE_FILE("assembly.c")

FILE *outputFilePtr = NULL;
extern int semanticErrorFlag;

static const char *TEMP_ASM_FILE = "build/test/temp_test.ass";
static const char *TEMP_BIN_FILE = "build/test/temp_test.out";

void setUp(void) {
    outputFilePtr = stdout;
    semanticOutputFile = stdout;
    semanticErrorFlag = 0;
    syntaxTree = NULL;
    initCodinterGenerator();
}

void tearDown(void) {
    remove(TEMP_ASM_FILE);
    remove(TEMP_BIN_FILE);
}

/*
 * Helper principal:
 * 1. Escanea, parsea y valida semánticamente el código C-TDS.
 * 2. Genera el código intermedio y el archivo Assembler x86-64 (.ass).
 * 3. Ensambla y enlaza el archivo .ass con GCC para verificar que la sintaxis x86-64 sea 100% válida.
 * 4. Ejecuta el binario compilado y devuelve el código de salida (retorno de main en %rax).
 */
static int compilar_ensamblar_y_ejecutar(const char *codigo_ctds) {
    YY_BUFFER_STATE buffer = yy_scan_string(codigo_ctds);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, yyparse(), "Fallo en la etapa de Parsing (yyparse)");

    SymbolTab *st = initializeSymbolTab();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, analyzeAst(syntaxTree, st), "Fallo en el Analisis Semantico");

    generateCodinterFromAST(syntaxTree);
    TEST_ASSERT_NOT_NULL_MESSAGE(getCodinterListHead(), "La lista de codigo intermedio esta vacia");

    FILE *asmOut = fopen(TEMP_ASM_FILE, "w");
    TEST_ASSERT_NOT_NULL_MESSAGE(asmOut, "No se pudo crear el archivo temporal .ass");

    generateAssembly(syntaxTree, getCodinterListHead(), asmOut);
    fclose(asmOut);
    yy_delete_buffer(buffer);

    /* Ensamblar con GCC (valida registros, etiquetas, alineación y sintaxis x86-64) */
    char cmd_gcc[256];
    snprintf(cmd_gcc, sizeof(cmd_gcc), "gcc -x assembler %s -o %s", TEMP_ASM_FILE, TEMP_BIN_FILE);
    int gcc_status = system(cmd_gcc);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gcc_status, "GCC fallo al ensamblar el codigo .ass generado");

    /* Ejecutar el binario nativo generado y capturar el valor de retorno (%rax) */
    char cmd_run[256];
    snprintf(cmd_run, sizeof(cmd_run), "./%s", TEMP_BIN_FILE);
    int run_status = system(cmd_run);
    TEST_ASSERT_TRUE_MESSAGE(WIFEXITED(run_status), "El binario ensamblado termino de forma anormal (crash/segfault)");

    return WEXITSTATUS(run_status);
}

static int archivo_asm_contiene(const char *subcadena) {
    FILE *f = fopen(TEMP_ASM_FILE, "r");
    if (!f) return 0;
    char linea[256];
    int encontrado = 0;
    while (fgets(linea, sizeof(linea), f)) {
        if (strstr(linea, subcadena) != NULL) {
            encontrado = 1;
            break;
        }
    }
    fclose(f);
    return encontrado;
}


void test_assembly_aritmetica_suma_resta_multiplicacion_y_negacion(void) {
    const char *codigo =
        "int main() {\n"
        "    int a, b, c;\n"
        "    a = 10;\n"
        "    b = 4;\n"
        "    c = (a + b) * 3 - (-b);\n" // (10 + 4) * 3 - (-4) = 42 + 4 = 46 
        "    return c;\n"
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(46, resultado);
    TEST_ASSERT_TRUE(archivo_asm_contiene("addq"));
    TEST_ASSERT_TRUE(archivo_asm_contiene("imulq"));
    TEST_ASSERT_TRUE(archivo_asm_contiene("negq"));
}

void test_assembly_division_y_modulo_con_cqto_idivq(void) {
    const char *codigo =
        "int main() {\n"
        "    int x, cociente, resto;\n"
        "    x = 29;\n"
        "    cociente = x / 5;\n" /* 5 */
        "    resto = x % 5;\n"    /* 4 */
        "    return cociente * 10 + resto;\n" /* 54 */
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(54, resultado);
    TEST_ASSERT_TRUE(archivo_asm_contiene("cqto"));
    TEST_ASSERT_TRUE(archivo_asm_contiene("idivq"));
}

void test_assembly_operadores_relacionales_y_logicos(void) {
    const char *codigo =
        "int main() {\n"
        "    int a, b, res;\n"
        "    boolean cond1, cond2;\n"
        "    a = 15;\n"
        "    b = 20;\n"
        "    res = 0;\n"
        "    cond1 = (a < b) && (b > 10);\n"
        "    cond2 = !(a == b) || false;\n"
        "    if (cond1 && cond2) {\n"
        "        res = 77;\n"
        "    }\n"
        "    return res;\n"
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(77, resultado);
    TEST_ASSERT_TRUE(archivo_asm_contiene("setl"));
    TEST_ASSERT_TRUE(archivo_asm_contiene("setg"));
    TEST_ASSERT_TRUE(archivo_asm_contiene("sete"));
}

void test_assembly_control_de_flujo_if_else_anidado(void) {
    const char *codigo =
        "int main() {\n"
        "    int x, r;\n"
        "    x = 10;\n"
        "    if (x > 20) {\n"
        "        r = 1;\n"
        "    } else {\n"
        "        if (x == 10) {\n"
        "            r = 25;\n"
        "        } else {\n"
        "            r = 2;\n"
        "        }\n"
        "    }\n"
        "    return r;\n"
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(25, resultado);
}

void test_assembly_ciclo_while_iterativo(void) {
    const char *codigo =
        "int main() {\n"
        "    int i, suma;\n"
        "    i = 1;\n"
        "    suma = 0;\n"
        "    while (i < 11) {\n"
        "        suma = suma + i;\n"
        "        i = i + 1;\n"
        "    }\n"
        "    return suma;\n" // Suma de 1 a 10 = 55 
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(55, resultado);
}

void test_assembly_llamada_a_funciones_y_pasaje_de_parametros_abi(void) {
    const char *codigo =
        "int calcular(int a, int b, int c) {\n"
        "    int temp;\n"
        "    temp = (a * b) - c;\n"
        "    return temp;\n"
        "}\n"
        "int main() {\n"
        "    int resultado;\n"
        "    resultado = calcular(6, 7, 12);\n" //42 - 12 = 30 
        "    return resultado;\n"
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(30, resultado);
    TEST_ASSERT_TRUE(archivo_asm_contiene("call calcular"));
}

void test_assembly_recursividad_y_aislamiento_de_stack_frame(void) {
    const char *codigo =
        "int factorial(int n) {\n"
        "    int res;\n"
        "    if (n == 1) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        res = n * factorial(n - 1);\n"
        "        return res;\n"
        "    }\n"
        "}\n"
        "int main() {\n"
        "    return factorial(5);\n" // 5! = 120 
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(120, resultado);
}

void test_assembly_variables_globales_direccionamiento_rip(void) {
    const char *codigo =
        "int acumulador;\n"
        "void incrementar(int valor) {\n"
        "    acumulador = acumulador + valor;\n"
        "    return;\n"
        "}\n"
        "int main() {\n"
        "    acumulador = 10;\n"
        "    incrementar(15);\n"
        "    incrementar(8);\n"
        "    return acumulador;\n" // 10 + 15 + 8 = 33 
        "}\n";

    int resultado = compilar_ensamblar_y_ejecutar(codigo);
    TEST_ASSERT_EQUAL_INT(33, resultado);
    TEST_ASSERT_TRUE(archivo_asm_contiene(".comm acumulador, 8, 8"));
    TEST_ASSERT_TRUE(archivo_asm_contiene("acumulador(%rip)"));
}
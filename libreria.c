#include <stdio.h>

int get_int() {
    int x;
    printf("Ingrese un entero: ");
    scanf("%d", &x);
    return x;
}

void printInt(int x) {
    printf("Resultado impreso por printInt: %d\n", x);
}
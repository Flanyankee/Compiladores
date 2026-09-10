# Compiladores
Proyecto Compiladores

## Integrantes:
Valentin Pastre Thuer, Francisco Gribaudo Re, Francisco Manuel Andreani

- Para generar el scanner con flex
`flex extended-language.lex`
- Para generar la tabla de parsing
`bison -vd extended-language.y`
- Para compilar el parser
`gcc -I./headers sources/* extended-language.tab.c lex.yy.c -lfl -o nombre-ejecutable`
- Para graficar con dot
`./nombre-ejecutable < text.txt` `dot -Tpng syntax-tree.dot -o nombre-imagen.png`
- Se crea un archivo symbol-tab.txt con la tabla de simbolos.

# Ejercicio 1, extención de gramática de expresiones
```
S -> Type Id () {P}
P -> E''' E'
R -> return; | return E;
E''' -> Type Id; E''' | Type Id = E; E''' | λ
E'' -> Id = E; | R
E' -> E'' E' | λ
E -> E + E
   | E * E
   | (E)
   | E && E
   | E || E
   | Num
   | Bool
   | Id
Id -> (a...z)Id' | (A...Z)Id'
Id' -> (a...z)Id' | (A...Z)Id´ | (0...9)Id' | λ
Bool -> true|false
Num -> -Num' | Num'
Num' -> (0...9) Num''
Num'' -> (0...9) Num'' | λ
Type -> int|bool|void
```
# Ejercicio 2, definiciones de estructuras
```
ID: (A...Z|a...z)(0...9|a...z|A...Z)*
CTTE: (0...9)+| True |False
OP: + | * | || | && | =
DELIM: '(' | ')' | '{' | '}'
RW: "main"
```

# Compiladores
Proyecto Compiladores

- Para generar el scanner con flex
`flex extended-language.lex`
- Para generar la tabla de parsing
`bison -vd extended-launguage.y`
- Para compilar el parser
`gcc extended-language.tab.c lex.yy.c -lfl -o nombre-ejecutable`

# Ejercicio 1, extención de gramática de expresiones
```
S -> Type Id () {P}
P -> E''' E' R
R -> return; R | return E; R | λ
E''' -> Type Id; E''' | Type E'' E''' | λ
E'' -> Id = E;
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

# Compiladores
Proyecto Compiladores

# Ejercicio 1, extención de gramática de expresiones

S -> Type Id () {P}
P -> E''' E' R
R -> return; | return E; | λ
E''' -> Type Id; | Type E'' | E''' E'''
E'' -> Id = E;
E' -> E'' | E'' E'
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

# Ejercicio 2, definiciones de estructuras

ID: (A...Z|a...z)(0...9|a...z|A...Z)*
CTTE: (0...9)+| True |False
OP: + | * | || | && | =
DELIM: '(' | ')' | '{' | '}'
RW: "main"

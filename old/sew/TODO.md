Rq: les cond de select... sont utilisées avant les blocks if, pas après -> 
       il faudrait une dépendance pour le calcul des cond

Attention aux stores, avec contraintes sur les registres

class A
    size := 0
    data ~= ___rep_type PI8, sb
    def sb
       size
    def init( s ) : size( s + 1 )

-> l'initialisation se fait... sans connaître la taille.
    Prop 1: cst() de taille inconnue puis on assigne en calculant les offsets au fur et à mesure
    On complète lat taille pendant la génération de code

Prop pour parse/find des méthodes:
* on met systématiquement self dans les variables catchées...
* mais lorsqu'on parse les méthodes, met 0 dedans
* quand on vient les chercher, on complète la donnée

Remarque: 
    toto := 10
    class A
        def f # -> la variable f fait référence à l'instance de A
        def g
        ... := toto # -> catche toto
    -> on ne peut a priori pas stocker f dans le type...
        Rq: le type dépend de Class, qui dépend bien des variables catchées
        !! les variables catchées peuvent être de différents type lors d'appels différents avec les même types
            Exemple :
                a ~= [ SI32, SI64 ][ val ]
                class A
                    def foo
                        a # 
        
!!! reassign devrait demander à mettre le résultat dans l'espace qui existe déjà
  a = b + c # l'espace mémoire pour b + c doit lui être proposé
  def f a, b; return a + b; c = f A, B # dans le cas de matrices, ça implique des copies
    -> prop 1:
        def f a, b
            if return_target isa Matrix
                return_target.resize ...
                return_target[ ... ] = ...
        A = f ... # -> return_target = A
        c := f(..) + ... # -> return_target = void
        
        signifie que tous les appels se font avec Expr& en paramètre
        
    -> prop 2: évaluation paresseuse
            
  def f a, b; return a + b; c := f A, B # on doit pouvoir repérer le cas en comptant les refs
    
À faire

* destruction des objets
* garbage collection
* reassign, copy, ... avec les types
* factorisation des opérations similaires


a := Ptr[Item]( 0 )
while ...
   a = Item( a, 123 )
   
Dans le while, on augmente la taille extérieure de la pile (la pile extérieure peut être gérée par un pointeur dans la pile)
On détruit cette partie là d'un bloc quand la variable n'est plus accessible

Rq: si on sauve un pointeur de la pile vers une zone gérée par le tas
  -> à tester

Pb philosophique : toutes les instructions agissent sur des zones mémoire
- on pourrait avoir add( room, a, b ) qui serait un set_val( room, add( a, b ) -> à ce moment là, autant faire un self_add( room, b )
- comment dans ce cas repérer que deux instructions sont égales ? Avec simplify

Autre solution : 
- un arbre de manipulation de zones mémoires en premier niveau
- un arbre SSA "pur" pour la génération de code.                               

-> syscall &p devient ssa
    (assign room_xx) -dep-> syscall room_xx -dep-> (les assign suivant)
-> une Var, c'est une position en mémoire + la valeur à cette position
-> un pointeur 
    
Pour les conditions :
- l'idée du assign_to (instructions conditionnelles) est elle vraiment SSA ?

Pb en cours : le conditions apparaissent comme des dépendances dans les blocks alors qu'une fois mises dans les blocks, on n'en a plus besoin
  -> prop 1 : on remplace les inp par des instructions bidon
  -> prop 2 : les blocks sont représentés pas des instructions


Type de taille variable
  Ex1:
    class Directory
        class Item
            name ~= CString
            ptr  ~= Inode
        icon ~= CString[ atomic = true ]
        data ~= Item{...}
        end ::= CompactString ""
        
    class CompactString
        size ~= CompactUnsigned
        data ~= repeat[ num = size ]
            PI8 
        def reassign val
            size = val.size
            memcpy &data, val.ptr, val.size
            
    -> oblige à garder les pointeurs sur les parents. Par exemple
       d := Directory()
       d.data[ 1 ].name = "toto" -> ok s'il ne faut pas faire de réallocation
         
  Question: comment faire des zones de "délestage" communes à plusieurs
    class Delestage
        a ~= CString
        b ~= CString
        r ~= CUnsigned
        d ~= repeat[ size = r - sizeof( a ) - sizeof( b ) ]
    ou
    
    class Delestage
        a ~= CString
        b ~= CString
        r ~= CUnsigned
        d ~= repeat[ size = r - sizeof( a ) - sizeof( b ) ]
  
  Si différentes possibilités
    class Example
        class A
            d ~= Vec[SI32,16]
        class B
            d ~= SI32
        type ~= PI8
        data ~= union
            a ~= A when type < 10
            b ~= B
        
  Utilisations de if, for, while, ... ?
    class Directory
        icon ~= CString
        data ~= while true
            name ~= CString
            if name == ""
                break
            inode ~= Inode

    d := Directory()
    d.
            
    mais pourquoi ne pas écrire
    
        while true
            if rand()
            name ~= CString
            if name == ""
                break
            inode ~= Inode
    
    
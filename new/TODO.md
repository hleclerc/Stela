À faire
* destruction des objets
* garbage collection
* slice
* simplification de ptr - ptr
* +, -, ...
* types de base (SI32, ...)
* reassign, copy, ... avec les types
* factorisation des opérations similaires
* Phi avec des références

* affichage de graph
* génération de code

a := Ptr[Item]( 0 )
while ...
   a = Item( a, 123 )
   
Dans le while, on augmente la taille extérieure de la pile (la pile extérieure peut être gérée par un pointeur dans la pile)
On détruit cette partie là d'un bloc quand la variable n'est plus accessible

Rq: si on sauve un pointeur de la pile vers une zone gérée par le tas
  -> à tester


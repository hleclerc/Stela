� faire
* destruction des objets
* slice
* simplification de ptr - ptr
* +, -, ...
* types de base (SI32, ...)
* reassign, copy, ... avec les types
* Phi
  ->


a := Ptr[Item]( 0 )
while ...
   a = Item( a, 123 )
   
Dans le while, on augmente la taille ext�rieure de la pile (la pile ext�rieure peut �tre g�r�e par un pointeur dans la pile)
On d�truit cette partie l� d'un bloc quand la variable n'est plus accessible

Rq: si on sauve un pointeur de la pile vers une zone g�r�e par le tas
  -> � tester


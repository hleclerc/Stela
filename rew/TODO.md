� faire
* code avec conditionnel
  -> syscall conditionnel avec select ?
  -> permettrait de tout passer dans les if
  -> les d�pendances passeraient aussi dans les if
  -> select avec des d�pendances ? SelectDep
* destruction des objets
* garbage collection
* slice
* +, -, ...
* reassign, copy, ... avec les types
* factorisation des op�rations similaires
* Phi avec des r�f�rences

a := Ptr[Item]( 0 )
while ...
   a = Item( a, 123 )
   
Dans le while, on augmente la taille ext�rieure de la pile (la pile ext�rieure peut �tre g�r�e par un pointeur dans la pile)
On d�truit cette partie l� d'un bloc quand la variable n'est plus accessible

Rq: si on sauve un pointeur de la pile vers une zone g�r�e par le tas
  -> � tester

Pb philosophique : toutes les instructions agissent sur des zones m�moire
- on pourrait avoir add( room, a, b ) qui serait un set_val( room, add( a, b ) -> � ce moment l�, autant faire un self_add( room, b )
- comment dans ce cas rep�rer que deux instructions sont �gales ? Avec simplify

Autre solution : 
- un arbre de manipulation de zones m�moires en premier niveau
- un arbre SSA "pur" pour la g�n�ration de code.                               

-> syscall &p devient ssa
    (assign room_xx) -dep-> syscall room_xx -dep-> (les assign suivant)
-> une Var, c'est une position en m�moire + la valeur � cette position
-> un pointeur 
    
Pour les conditions :
- l'id�e du assign_to (instructions conditionnelles) est elle vraiment SSA ?

Pb en cours : le conditions apparaissent comme des d�pendances dans les blocks alors qu'une fois mises dans les blocks, on n'en a plus besoin
  -> prop 1 : on remplace les inp par des instructions bidon
  -> prop 2 : les blocks sont repr�sent�s pas des instructions

  
  
  
  
  
Concours scientifique -> pas de risque d'atteinte.
- droit � 20% pour le concours scientifique

- SARL -> 
- SAS -> soci�t� pas action simplifi�
- 
- 






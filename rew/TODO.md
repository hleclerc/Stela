À faire
* code avec conditionnel
  -> syscall conditionnel avec select ?
  -> permettrait de tout passer dans les if
  -> les dépendances passeraient aussi dans les if
  -> select avec des dépendances ? SelectDep
* destruction des objets
* garbage collection
* slice
* +, -, ...
* reassign, copy, ... avec les types
* factorisation des opérations similaires
* Phi avec des références

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

  
  
  
  
  
Concours scientifique -> pas de risque d'atteinte.
- droit à 20% pour le concours scientifique

- SARL -> 
- SAS -> société pas action simplifié
- 
- 






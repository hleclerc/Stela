Important pour la suite:
- layers de variables
  -> Expr/Inst pour du SSA pur (pointer_on renvoie le pointeur d'une variable dans un état donné)
  -> Ref pour des variables non typées
      avec gestion des conditions (static cond := Vec[Expr]) ?
      des snapshots (pour les while, les fonctions outline) ?
  -> Var pour des variables typées
      Rq: quand on fait break, la couche Ref invalide les instructions suivantes, les destructeurs sont appelés ensuite dans la couche Var
-


Ce qui est mieux que Rust:
- stack closure ne sont pas première classe en Rust alors qu'elles le sont avec Stela
- notations chiante (ex: les closures)
- les méthodes sont définies à l'extérieur, alors que ça devrait être la première info qui arrive
- generic limité -> not metaprogrammation


Objets Ref[ T ]
-> idée de base: un pointeur sur une variable, qui empêche la variable d'être détruite 
-> oblige à gérer une table de correspondance côté interpréteur/compilateur, vers les cptr
-> pour être conservateur au niveau du compilateur, il faudrait ne décrémenter les ref_count que si aucun lien n'est possible vers la variable.
- prop: a basic ptr type (null_ptr) and a basic ref type (null_ref) pour éviter les "add_dep" manuels, mais fonctionner sans Ref[ T ] qui demande des prérequis (VarArgs, ...)
- autre solution : on catch les appels genre Ref[ T ] pour mettre directement le bon type (pourrait aussi se faire aussi en disant qu'une référence à une classe, template ou pas, renvoie le type... "Ref" demande Ref sans arguments, alors )

Remarques sur ce qui est connu à la compilation ou pas
- idée de base : faire un nouveau type force à faire des compilations différentes. Mais on pourrait faire imposer qu'une variable soit connue par exemple avec un "known".
- le known pourrait fonctionner pour retrouver le type d'un MO, ...
- on pourrait aussi accepter des alternatives. Par exemple, cond ? 1 : "a" renvoie une variable avec flag pour dire le type... bof : ralentira tout
- 

Permettre de défragmenter la mémoire ?
-> oblige d'interdire de transformer les pointeurs en entiers "basiques"
-> potentiellement lent sur les listes chainées
-> pourrait être utile pour "l'allocateur par taille" qui risque de fragmenter un peu plus 

Problème des références attrapées. Ex: def f() ( a := [10 20]; return a[ 0 ] )
-> serait une solution pour éviter les copies de variable
-> permettrait de gérer complétement la durée de vie des ref, y compris pour les closures ! 
=> idée : quand on appelle une fonction, on réserve de la place pour les variables attrapées (le f() va réserver de la place pour a := [10 20])
    - pb 1 : dans certains cas, on aura du mal à savoir combien il faut de place.
        - Ex : def f( n ) v := stack_alloc( n ); return v[ 0 ]...
        - solution 1 : s'il y a du stack_alloc, on fait des piles segmentées (e.g. des pointeurs)
        - solution 2 : on repère les cas où on peut trouver les n _avant_ de faire l'allocation. Si pas possible, on refuse de sortir des réfèrences 
    - pb 2 ; récursivivité. Ex: def f( t ) ( a := [10 t]; return f( a ) )
        - il faudrait augmenter la taille de la pile... de la fonction appelante
        - prop : on repère les cas où la pile de la fonction appelante peut grossir pendant un appel. Dans ce cas, la fonction appelée est lancée dans une nouvelle pile.
        
        
Piles qui évoluent en parallèle
- peut fonctionner comme en go: la réservation se fait en comparant à de la fin de mémoire réservée.
- rq: on pourrait aussi ne faire qu'une pile, avec une gestion de la fragmentation : on enregistre les causes de fragmentation, et quand on déroule, on saute le vide...
- autre solution : on fait de l'allocation dynamique quand une pile ne fonctionne plus
    
Faut-il créer de nouveaux types à chaque fois qu'on crée une fonction ? Si non :
- on aura un système plus dynamique (possible de faire listes de fonctions, ...)
    -> exemple pour du code rentré de façon dynamique
- mieux pour l'occupation mémoire (les infos liées aux types ne sont jamais effacées de la mémoire)
- on devra mettre un système embarqué pour voir si telle ou telle fonction est compilée, ce qu'elle retourne, etc...
    Exemple : sort( a, f )
        -> pour compiler sort, il faut que le type de retour de f( x, y ) soit connu pour x, y donné... ce qui ne fonctionne pas avec juste un pointeur sur tok_data
    Sol 1 : sort( a, f! ) pour que la fonction soit compilée avec un préambule qui teste les valeurs de f pour voir si nécessaire de compiler qq chose, ... 
        Rq : on pourrait chercher à automatiser la nécessité du ! ou pas... mais il faudrait trouver le niveau correct de granularité.
        Rq : sort( a, f! ) pourrait sortir des types différents fonction du f... du coup on ne peut pas compiler les fonctions qui utilisent sort tant qu'on ne connait pas f.
        
    Autre remarque: les defs sont forcément dans des scopes statiques (parce qu'elles peuvent elles-mêmes contenir des scopes statiques).





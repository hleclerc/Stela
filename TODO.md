



Ce qui est mieux que Rust:
- stack closure ne sont pas premiËre classe en Rust alors qu'elles le sont avec Stela
- notations chiante (ex: les closures)
- les mÈthodes sont dÈfinies ‡ l'extÈrieur, alors que Áa devrait Ítre la premiËre info qui arrive
- generic limitÈ -> not metaprogrammation


Objets Ref[ T ]
-> idÈe de base: un pointeur sur une variable, qui empÍche la variable d'Ítre dÈtruite 
-> oblige ‡ gÈrer une table de correspondance cÙtÈ interprÈteur/compilateur, vers les cptr
-> pour Ítre conservateur au niveau du compilateur, il faudrait ne dÈcrÈmenter les ref_count que si aucun lien n'est possible vers la variable.
- prop: a basic ptr type (null_ptr) and a basic ref type (null_ref) pour Èviter les "add_dep" manuels, mais fonctionner sans Ref[ T ] qui demande des prÈrequis (VarArgs, ...)
- autre solution : on catch les appels genre Ref[ T ] pour mettre directement le bon type (pourrait aussi se faire aussi en disant qu'une rÈfÈrence ‡ une classe, template ou pas, renvoie le type... "Ref" demande Ref sans arguments, alors )

Remarques sur ce qui est connu ‡ la compilation ou pas
- idÈe de base : faire un nouveau type force ‡ faire des compilations diffÈrentes. Mais on pourrait faire imposer qu'une variable soit connue par exemple avec un "known".
- le known pourrait fonctionner pour retrouver le type d'un MO, ...
- on pourrait aussi accepter des alternatives. Par exemple, cond ? 1 : "a" renvoie une variable avec flag pour dire le type... bof : ralentira tout
- 

Permettre de dÈfragmenter la mÈmoire ?
-> oblige d'interdire de transformer les pointeurs en entiers "basiques"
-> potentiellement lent sur les listes chainÈes
-> pourrait Ítre utile pour "l'allocateur par taille" qui risque de fragmenter un peu plus 

ProblËme des rÈfÈrences attrapÈes. Ex: def f() ( a := [10 20]; return a[ 0 ] )
-> serait une solution pour Èviter les copies de variable
-> permettrait de gÈrer complÈtement la durÈe de vie des ref, y compris pour les closures ! 
=> idÈe : quand on appelle une fonction, on rÈserve de la place pour les variables attrapÈes (le f() va rÈserver de la place pour a := [10 20])
    - pb 1 : dans certains cas, on aura du mal ‡ savoir combien il faut de place.
        - Ex : def f( n ) v := stack_alloc( n ); return v[ 0 ]...
        - solution 1 : s'il y a du stack_alloc, on fait des piles segmentÈes (e.g. des pointeurs)
        - solution 2 : on repËre les cas o˘ on peut trouver les n _avant_ de faire l'allocation. Si pas possible, on refuse de sortir des rÈfËrences 
    - pb 2 ; rÈcursivivitÈ. Ex: def f( t ) ( a := [10 t]; return f( a ) )
        - il faudrait augmenter la taille de la pile... de la fonction appelante
        - prop : on repËre les cas o˘ la pile de la fonction appelante peut grossir pendant un appel. Dans ce cas, la fonction appelÈe est lancÈe dans une nouvelle pile.
        
        
Piles qui Èvoluent en parallËle
- peut fonctionner comme en go: la rÈservation se fait en comparant ‡ de la fin de mÈmoire rÈservÈe.
- rq: on pourrait aussi ne faire qu'une pile, avec une gestion de la fragmentation : on enregistre les causes de fragmentation, et quand on dÈroule, on saute le vide...
- autre solution : on fait de l'allocation dynamique quand une pile ne fonctionne plus
    
Faut-il crÈer de nouveaux types ‡ chaque fois qu'on crÈe une fonction ? Si non :
- on aura un systËme plus dynamique (possible de faire listes de fonctions, ...)
    -> exemple pour du code rentrÈ de faÁon dynamique
- mieux pour l'occupation mÈmoire (les infos liÈes aux types ne sont jamais effacÈes de la mÈmoire)
- on devra mettre un systËme embarquÈ pour voir si telle ou telle fonction est compilÈe, ce qu'elle retourne, etc...
    Exemple : sort( a, f )
        -> pour compiler sort, il faut que le type de retour de f( x, y ) soit connu pour x, y donnÈ... ce qui ne fonctionne pas avec juste un pointeur sur tok_data
    Sol 1 : sort( a, f! ) pour que la fonction soit compilÈe avec un prÈambule qui teste les valeurs de f pour voir si nÈcessaire de compiler qq chose, ... 
        Rq : on pourrait chercher ‡ automatiser la nÈcessitÈ du ! ou pas... mais il faudrait trouver le niveau correct de granularitÈ.
        Rq : sort( a, f! ) pourrait sortir des types diffÈrents fonction du f... du coup on ne peut pas compiler les fonctions qui utilisent sort tant qu'on ne connait pas f.
        
    Autre remarque: les defs sont forcÈment dans des scopes statiques (parce qu'elles peuvent elles-mÍmes contenir des scopes statiques).

Contreparties de faire un type diffÈrent pour chaque callable et combinaisons :
- ‡ chaque fois qu'on demande une fonction, il faut faire un nouveau type de surdef_list

class BegVarargsItem[ T0, TN ]
    item ~= T0
    next ~= TN
class EndVarargsItem
    ...

class SurdefList[ varargs ] varargs contient dans les T0, les pointeurs vers les callables



Class Type[ data_ptr ] -> 

Question : dans quel scope lancer les opÈrations sur les paramËtres templates ? Prop : on laisse ‡ l'utilisateur la responsabilitÈ de laisser  un equal disponible au niveau suffisant.

PoblËme : comment faire de l'allocation dynamique sur la pile ?
- prop : primitive stack_alloc, ‡ appeler avec des inlines. Par ex: v := 
    
Avec IR rÈcursive
- si un bloc ne contient qu'une instruction, c'est celle que l'on prend
- sinon, une instruction IR_TOK_BLOCK permet de gÈrer les sÈquences (-> liste d'offsets terminÈe par 0)
- 
- Comment connaitre la taille des codes IR ? Rq: on n'est intÈressÈ par cette donnÈe que pour les Callables. On peut faire un DelayedParse uniquement pour avoir un offset de fin dans les paramËtres de callable
- 
- ProblËme : faut-il envoyer nstring relocation quand on envoie un code sous forme IR ?



¿ quoi servent les pointeurs sur des Expr ?
  - ex: si on fait ptr( slice( a, 32, 64 ) ), est-ce que Áa devrait donner la mÍme chose que ptr( a ) + 32 ?
      -> a priori oui
      -> Áa veut dire qu'il faut garder le slice 


      
Pb: cst_data( 5, 17 ) -> devrait renvoyer un pointeur sur les donnÈes dÈcalÈes en mÈmoire...
  -> c'est possible si on fait les version dÈcalÈes des Cst::data pour les 8 cas possibles (‡ la demande)
      

Les classes et les type dÈmarrent avec les bons types

Rq: on pourrait stocker les types avec des Expr mais il faudrait avoir la bonne valeur dËs le dÈbut
On pourrait utiliser un Expr *

    
Toto[ ... ] -> comment sauver les ... ?
    -> avec constity si possible (sinon on fait une copie et on constity)
    
Ref[] -> proposition: un utilise un null_ref et un map<ST,Vec<Var> > c√¥t√© interpr√©teur



� quoi servent les pointeurs sur des Expr ?
  - ex: si on fait ptr( slice( a, 32, 64 ) ), est-ce que �a devrait donner la m�me chose que ptr( a ) + 32 ?
      -> a priori oui
      -> �a veut dire qu'il faut garder le slice 


      
Pb: cst_data( 5, 17 ) -> devrait renvoyer un pointeur sur les donn�es d�cal�es en m�moire...
  -> c'est possible si on fait les version d�cal�es des Cst::data pour les 8 cas possibles (� la demande)
      

Les classes et les type d�marrent avec les bons types

Rq: on pourrait stocker les types avec des Expr mais il faudrait avoir la bonne valeur d�s le d�but
On pourrait utiliser un Expr *

    
Toto[ ... ] -> comment sauver les ... ?
    -> avec constity si possible (sinon on fait une copie et on constity)
    
Ref[] -> proposition: un utilise un null_ref et un map<ST,Vec<Var> > côté interpréteur



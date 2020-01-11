# codageVoixImage
NOZAHIC Morvan
PELOUS Thomas

Nous avons travaillé sur le codage d'une vidéo.
Tout d'abord, nous avons compressé la vidéo d'origine avec targz, la taille après compression était encore assez élevée.
Nous avons utilisé le programme fournit pour faire une première compression et la taille après la compression targz était bien plus petite.
Cela est dû au fait que la compression fournit par le programme conserve la première image avec les couleurs puis ensuite le reste de la vidéo est encodé avec les nuances des mouvements.

En parcourant chaque pixel de chaque bloc, on calcule une moyenne des différences en valeur absolue entre tous les pixels du bloc courant (bloc source) et du bloc précédent (bloc référence).

On récupère ensuite la moyenne minimale des différences sous forme de coordonnées moyennes. Ces dernières constituent les coordonnées du vecteur déplacement. 

Ensuite, nous avons tenté de trouver des nouvelles fonctions pour la prédiction de mouvement.
Nous avons choisi les fonctions logarithmique et racine carrée. 
Nous avons eu du mal à exploiter nos résultats car nous avons tardé à avoir accès au courbes tracées à cause d'un mauvais fichier téléchargé.
Un des nouveaux prédicteurs avait quelques défauts et n'était pas monotone lors du passage par l'origine ce qui entrainnait des défauts lors de la compression.

Le résultat de la première compression avec l'estimation de mouvement ne diminue pas la taille du fichier mais par contre elle le rend plus compressible.
Les estimations de mouvement permettent aux algorithmes tel que targz de pouvoir compresser les motifs qui se repettent et ainsi être plus efficace.
On a noté une taille après compression targz qui était de lordre de 25% par rapport à la taille du fichier originale.





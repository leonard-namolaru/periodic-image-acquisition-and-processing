### Programmation embarquée (Heptagon) - Contrôler un véhicule autonome dans une ville virtuelle
![prog-sync-img.png](prog-sync-img.png.png)
> :school: **Lieu de formation :** Université Paris Cité, Campus Grands Moulins (ex-Paris Diderot)
> 
> :books: **UE :** Programmation embarquée 
> 
> :pushpin: **Année scolaire :** M2
> 
> :calendar: **Date :** Décembre 2022 

#### Description
Un programme synchrone contrôlant un dispositif physique simplifie mais non trivial : un véhicule autonome dans une ville virtuelle en deux dimensions. 
Le but est donc de franchir toutes les étapes d’un parcours préétabli tout en respectant un certain nombre de contraintes 
(limitations de vitesse, feux rouges, évitement des obstacles). Le véhicule est équipé d’actuateurs et de capteurs.

_Remarque :_ Heptagon est un langage synchrone universitaire très proche du langage industrielle SCADE 6.

#### Travail effectué
Compte tenu du dispositif décrit ci-dessus, notre tâche était d’écrire le contrôleur du véhicule de manière à ce qu’il utilise les entrées reçues par les capteurs afin de permettre à la voiture de parcourir l’itinéraire prédéfini de manière autonome. Notre implémentation est principalement basée sur un correcteur PID. 

Le code du contrôleur se trouve sur ce GitHub, accompagné d’un rapport expliquant l’implémentation.

#### Ressources supplémentaires
- [Rapport](Rapport_FR.pdf)

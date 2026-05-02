## Ruins of Heastia v1.0

>Cette release marque l'aboutissement de 2 mois de développement intensif. Elle couvre la création des briques fondamentales du moteur C++ personnalisé (GPC Engine) jusqu'à l'implémentation complète de la boucle de gameplay de gestion du Village et d'expédition.

<p align="center">
  <img width="60%" alt="affiche A4" src="https://github.com/user-attachments/assets/1ac52a4c-319c-4e1e-a2a5-871087338e0a" />
</p>

---

### Moteur & Architectutre
- Architecture ECS : Mise en place d'un modèle Entité-Composant-Système garantissant la modularité et la maintenabilité du code.

- Rendu Vulkan : Implémentation du pipeline graphique sous Vulkan. Intégration des ombres et d'un système de particules avancé entièrement calculé sur le GPU via des Compute Shaders.

- Moteur Audio : Développement du système de son natif et spatiale. Prise en charge des fichiers WAV, séparation des canaux (Music / SFX), gestion du looping et contrôle des volumes en temps réel via les paramètres.

- Asset Manager : Chargement optimisé des ressources (textures, spritesheets animées, shaders, audio).

- UI Builder Reactif : Création d'une API déclarative (UiBuilder) pour générer les interfaces complexes, incluant un système de Hitboxes strict, des ancrages dynamiques, et du data-binding (UiReactive) pour la mise à jour en temps réel.

- Sauvegarde : Implémentation d'arbre bianaire pour le système de sauvegarde.

- Easing : Implémentation d'un système de Tween, pratique pour créer de nombreux effet facilement avec des maths.

- FBX loader : Le moteur est en capacité de jouer des animations FBX.

--- 

### Gameplay : Le Village

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/dbc68a6f-de2c-4984-8f92-ea1328120a8f" />
</p>

- Construction & Placement : Système de grille spatiale permettant d'ajouter, déplacer ou stocker des bâtiments (Fermes, Maisons, Forge) avec gestion des collisions et callbacks liés aux instances.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/28140309-7b5e-4854-99c0-0dcb7c3ec3cf" />
</p>

- Boutique & Gestion de l'Inventaire : Suivi des ressources primaires (Bois, Pierre, Métal, Fibre, Nourriture) et du stockage des équipements et gemmes du village. Avec ces ressources vous pouvez acheter de nouveaux bâtiments et améliorer votre équipement dans la forge.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/e360efb0-ec8f-4790-aed0-e685fc8ecb84" />
</p>

- Sauvegarde & Paramètres : Sérialisation de l'état du village (ressources, placements) et gestion des paramètres utilisateur (sauvegarde des préférences audio).

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/bfbcffcb-bef3-4864-ad26-71f1b7da1d39" />
</p>

--- 

### Gameplay : L'Expédition

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/14ea38db-8b46-42ae-abbf-165a5a68a05f" />
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/0a61c36a-130e-49bd-87e1-350641ac1477" />
</p>

- Génération Procédurale : Création d'environnements uniques à chaque départ. La carte, la répartition des ennemis et des ressources sont générées dynamiquement pour assurer une rejouabilité infinie.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/db06da5e-a0de-4ebe-9bad-cb00a8cf2af2" />
</p>

- Récolte de Ressources : Exploration des niveaux permettant de collecter les matériaux et les gemmes qui serviront de retour au village pour l'artisanat et le développement.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/dc83eec0-ff4b-43b9-949b-19ec39440376" />
</p>

- Combats & Mobs : C'est ici que les statistiques de l'équipement forgé au village prennent tout leur sens.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/44b0309c-6c68-4cd5-9dcb-cd7a1352465c" />
</p>

---



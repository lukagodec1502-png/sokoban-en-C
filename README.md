# 📦 Moteur de Jeu Sokoban - BUT Informatique

Ce dépôt contient le code source de mon projet de développement d'un jeu Sokoban en langage C. Ce projet a été réalisé de A à Z dans le cadre de ma première année de BUT Informatique à l'IUT de Lannion.

## 🎯 Description du Projet
L'objectif principal était de concevoir un jeu de puzzle fonctionnel en ligne de commande. Le joueur doit y faire preuve de logique pour pousser des caisses sur des cibles spécifiques dans un entrepôt, sans rester bloqué. Ce projet m'a permis de consolider mes bases en programmation procédurale.

## 🎮 Modes de Jeu
Le programme intègre deux versions distinctes du jeu :
* **Version Manuelle (Classique) :** Le joueur contrôle les mouvements de manière interactive. Ce mode nécessite exclusivement le fichier `niveau1.sok`, qui contient la grille de jeu (la map).
* **Version Automatique :** Le jeu s'exécute de manière autonome en lisant une série d'instructions. Ce mode utilise le fichier `grille_a.dept`, qui contient la liste des mouvements à effectuer.

## 🛠️ Technologies Utilisées
* **Langage :** C
* **Environnement de développement :** Linux / Terminal
* **Outils :** GCC, Make, Git pour le versionning

## 🚀 Fonctionnalités Implémentées
* **Gestion de la carte :** Chargement et affichage de la grille de jeu à l'aide de tableaux 2D.
* **Logique de déplacement :** Implémentation des déplacements du personnage (Haut, Bas, Gauche, Droite) via les entrées utilisateur ou la lecture automatisée de fichiers.
* **Système de collisions :** Détection stricte des murs et gestion physique de la poussée des caisses.
* **Conditions de victoire :** Algorithme de vérification en temps réel de l'emplacement exact de toutes les caisses sur les zones de validation.

## 🎓 Compétences BUT Validées (Preuves Portfolio)
Ce projet constitue une trace concrète de mon apprentissage et valide directement plusieurs compétences clés de la formation :

* **Compétence 1 - Réaliser (Développer des applications informatiques simples) :** Conception complète du programme, de l'analyse du besoin jusqu'à l'exécution du code compilé sans erreurs[cite: 2].
* **Compétence 2 - Optimiser (Appréhender et construire des algorithmes) :** Utilisation de la logique algorithmique pour gérer les tableaux à double entrée, manipulation des pointeurs et optimisation de la gestion mémoire lors des déplacements[cite: 2].

## 📸 Aperçu du Projet
*(Remplacez ce texte par une capture d'écran du jeu fonctionnant dans votre terminal)*

## 💻 Installation et Exécution
Pour compiler et lancer le jeu sur un environnement Linux :

```bash
# Cloner le dépôt
git clone [https://github.com/lukagodec1502-png/nom-de-ton-repo.git](https://github.com/lukagodec1502-png/nom-de-ton-repo.git)

# Se rendre dans le dossier
cd nom-de-ton-repo

# Compiler le projet
make

# Lancer la version manuelle (avec la grille de jeu)
./sokoban niveau1.sok

# Lancer la version automatique (avec la liste des mouvements)
./sokoban grille_a.dept

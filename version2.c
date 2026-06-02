/**
* @file sokoban_v2.c
* @brief Programme du jeu de Sokoban - Version 2
* @details Cette version ajoute des fonctionnalités avancées par rapport à la V1 :
* - Zoom d'affichage (x1 à x3)
* - Historique des mouvements
* - Annulation du dernier coup (Undo)
* - Sauvegarde du replay (.dep)
* @author Luka Godec
* @version 1.2
* @date 30/11/2025
*/

/* --- INCLUSION DES BIBLIOTHÈQUES --- */
#include <termios.h> 
#include <unistd.h>  
#include <fcntl.h>   /* Contrôle des fichiers (pour kbhit) */
#include <stdbool.h> /* Type booléen */
#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  /* Manipulation de chaînes */

/* --- CONSTANTES --- */
#define TAILLE 12        /* Le plateau est toujours 12x12 */
#define LIGNE TAILLE     
#define COLONNE TAILLE   

#define CHOIX 50         /* Taille max nom de fichier */
#define NOM 46           /* Taille max nom sauvegarde */
#define MAX_DEPLACEMENTS 1000 /* Capacité de l'historique */

/* --- SYMBOLES DU JEU --- */
#define MUR '#'
#define CAISSE '$'
#define CIBLE '.'
#define SOKOBAN '@'
#define SOKOCIBLE '+'    /* Sokoban sur cible */
#define CAISSECIBLE '*'  /* Caisse sur cible */
#define AUTRE ' '        /* Case vide */

/* --- TOUCHES DE JEU --- */
#define AVANCER 'z'
#define RECULER 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define RECOMMENCER 'r'
#define ABANDONNER 'x'

/* --- NOUVELLES TOUCHES V2 --- */
#define ZOOM '+'         /* Agrandir le plateau */
#define DEZOOM '-'       /* Rétrécir le plateau */
#define UNDO 'u'         /* Annuler le dernier coup */

/* --- CODES INTERNES POUR L'HISTORIQUE (V2) --- */
/* Minuscules : Déplacement simple */
#define SGAUCHE 'g'
#define SHAUT 'h'
#define SBAS 'b'
#define SDROITE 'd'
/* Majuscules : Poussée de caisse (Indispensable pour l'Undo) */
#define SOKOGAUCHE 'G'
#define SOKOHAUT 'H'
#define SOKOBAS 'B'
#define SOKODROITE 'D'


/* --- VARIABLES GLOBALES --- */
char fichier[CHOIX];       /* Fichier en cours */
char fichierNiveau[CHOIX]; /* Copie pour le redémarrage */

int niveau; /* Numéro du niveau */

/* Position du joueur */
int sokobanX = -1;
int sokobanY = -1;    

/* Vecteur de direction */
int directionX = 0;
int directionY = 0;

/* --- TYPES --- */
typedef char t_plateau[LIGNE][COLONNE];          
typedef char t_tabDeplacement[MAX_DEPLACEMENTS]; /* Tableau stockant la suite de caractères (V2) */

/* Matrice mémoire pour les cibles */
bool cibles[LIGNE][COLONNE]; 

/* --- PROTOTYPES --- */
int kbhit(void);
void charger_partie(t_plateau plateau, char fichier[]);     
void enregistrer_partie(t_plateau plateau, char fichier[]);
void afficher_entete(char fichier[CHOIX], int nbDeplacements);
void afficher_plateau(t_plateau plateau, int zoom); /* Ajout paramètre zoom */
void deplacer(t_plateau plateau, char touche, int *zoom, t_tabDeplacement historique, int *nbDeplacements);
void jouer(t_plateau plateau, t_tabDeplacement historique, int *nbdp, char touche);
void initialiser_cibles(t_plateau plateau);
/* Nouvelles fonctions V2 */
void enregistrer_deplacements(t_tabDeplacement historique, int nb, char fic[]);
void memoriser_deplacements(char touche, t_tabDeplacement historique, int *nb, int pousseCaisse);
void annuler_deplacement(t_plateau plateau, t_tabDeplacement historique, int *nbdp);
bool gagne(t_plateau plateau);


/**
 * @brief Fonction principale V2
 */
int main(){
    t_plateau plateau;
    int zoom = 1; /* Niveau de zoom par défaut */
    t_tabDeplacement historique; /* Stockage des coups pour Undo/Replay */
    int nbDeplacements = 0;
    char touche = 0;

    /* Initialisation */
    system("clear");
    printf("Quel niveau voulez vous jouer?\n");
    scanf("%d", &niveau);
    getchar(); 
    
    sprintf(fichier, "niveau%d.sok", niveau);
    strcpy(fichierNiveau, fichier); 
    
    charger_partie(plateau, fichier);
    initialiser_cibles(plateau);

    system("clear");
    afficher_entete(fichier, nbDeplacements);
    afficher_plateau(plateau, zoom);

    /* Boucle de jeu */
    while(true){
        if (kbhit()){
            touche = getchar();
            
            /* On passe l'historique et le zoom à la fonction déplacer */
            deplacer(plateau, touche, &zoom, historique, &nbDeplacements);
            
            system("clear");
            afficher_entete(fichier, nbDeplacements);
            afficher_plateau(plateau, zoom);
            
            if (gagne(plateau)){
                printf("\nFélicitations ! Vous avez gagné la partie en %d déplacements.\n", nbDeplacements);
                
                /* Fonctionnalité  : Sauvegarde du replay en fin de partie */
                printf("Voulez-vous enregistrer le replay ? (o/n) : ");
                char choix;
                scanf(" %c", &choix); 
                if (choix == 'o'){
                    char nomFic[CHOIX];
                    printf("Nom du replay (.dep) : ");
                    scanf("%s", nomFic);
                    strcat(nomFic, ".dep");
                    enregistrer_deplacements(historique, nbDeplacements, nomFic);
                    printf("Replay sauvegardé.\n");
                }
                break;
            }
        }
    }
    return 0;
}


/**
 * @brief Affiche le plateau avec gestion du ZOOM (V2).
 * @details Utilise des boucles imbriquées pour répéter chaque caractère
 * 'zoom' fois horizontalement et verticalement.
 */
void afficher_plateau(t_plateau plateau, int zoom){
    for (int i = 0; i < LIGNE; i++){
        for(int zo=0; zo<zoom; zo++){ /* Répétition hauteur */
            for (int j = 0; j < COLONNE; j++){
                for(int zo2=0; zo2<zoom; zo2++){ /* Répétition largeur */
                    if (plateau[i][j] == CAISSECIBLE){
                        printf("$"); 
                    } 
                    else if (plateau[i][j] == SOKOCIBLE){
                        printf("@"); 
                    } 
                    else {
                        printf("%c", plateau[i][j]);
                    }
                }
            }
            printf("\n");
        }    
    }
}

/**
 * @brief Affiche l'entête enrichi des options V2.
 */
void afficher_entete(char fichier[CHOIX], int nbDeplacements){
    printf("Touches de déplacement : avancer:z ; reculer:s ; gauche:q ; droite:d\n");
    printf("Autres touches : abandonner:x ; recommencer:r ; Zoom: + - ; Annuler: u\n");
    printf("Fichier : %s\n", fichier);
    printf("Nombre de déplacements effectués : %d\n\n", nbDeplacements);
}

/**
 * @brief Contrôleur principal .
 * @details Gère les nouvelles touches : ZOOM, DEZOOM, UNDO.
 */
void deplacer(t_plateau plateau, char touche, int *zoom, t_tabDeplacement historique, int *nbDeplacements){
    int abandon = 0;
    int save = 0;
    char nom[NOM];

    /* Mise à jour position joueur */
    for (int i = 0; i < LIGNE; i++){
        for (int j = 0; j < COLONNE; j++){
            if (plateau[i][j] == SOKOBAN || plateau[i][j] == SOKOCIBLE){
                sokobanX = i;
                sokobanY = j;
            }
        }
    }
    
    directionX = 0;
    directionY = 0;

    switch (touche){
        case AVANCER: directionY = -1; break;
        case RECULER: directionY = 1;  break;  
        case GAUCHE:  directionX = -1; break; 
        case DROITE : directionX = 1;  break; 
        
        case ABANDONNER : {
            printf("\nVous avez choisi d'abandonner la partie.\n");
            printf("Veuillez confirmer votre demande (0:ANNULER, 1:OUI)\n");
            scanf("%d", &abandon);
            if (abandon == 1){
                /* Sauvegarde partie .sok */
                printf("Voulez-vous sauvegarder avant ? (0:NON, 1:OUI)\n");
                scanf("%d", &save);
                if (save == 1){
                    printf("Nom de la sauvegarde : ");
                    scanf("%s", nom);
                    char fichierSave[CHOIX];
                    snprintf(fichierSave, sizeof(fichierSave), "%s.sok", nom);
                    enregistrer_partie(plateau, fichierSave);
                    printf("Partie sauvegardée sous %s\n", fichierSave);
                }
                /* Sauvegarde Replay .dep (V2) */
                printf("Voulez-vous enregistrer le replay des touches ? (0:NON, 1:OUI)\n");
                int savePartie = 0;
                scanf("%d", &savePartie);
                if (savePartie == 1){
                    printf("Nom du replay (.dep) : ");
                    scanf("%s", nom);
                    strcat(nom, ".dep");
                    enregistrer_deplacements(historique, *nbDeplacements, nom);
                    printf("Replay sauvegardé");
                }              
                system("clear");
                printf("Fin du jeu. À bientôt !\n");
                exit(EXIT_SUCCESS);
            }
            break;
        }
        case RECOMMENCER : {
            char continuer;
            printf("\nRecommencer la partie ? (o:continuer / autre:annuler)\n");
            scanf(" %c", &continuer); 

            if (continuer == 'o' ){
                *nbDeplacements = 0;
                system("clear");
                charger_partie(plateau, fichierNiveau);
                initialiser_cibles(plateau);
            }
            return;
        }
        /* Gestion du Zoom V2 */
        case ZOOM : 
            if (*zoom < 3) (*zoom)++;
            break;
        case DEZOOM :
            if (*zoom > 1) (*zoom)--;
            break;
        
        /* Gestion de l'Undo V2 */
        case UNDO :
            annuler_deplacement(plateau, historique, nbDeplacements);
            break;
    }

    if (directionX != 0 || directionY != 0){
        jouer(plateau, historique, nbDeplacements, touche);
    }
}

/**
 * @brief Logique de jeu V2 avec Mémorisation.
 * @details Appelle memoriser_deplacements() après chaque mouvement réussi.
 */
void jouer(t_plateau plateau, t_tabDeplacement historique, int *nbdp, char touche){
    int nx = sokobanX + directionY;
    int ny = sokobanY + directionX;
    int nx2 = sokobanX + 2 * directionY; 
    int ny2 = sokobanY + 2 * directionX;
    
    if (nx < 0 || nx >= LIGNE || ny < 0 || ny >= COLONNE) return;

    char caseSuivante = plateau[nx][ny];

    if (caseSuivante == MUR) return;

    /* Cas Poussée */
    if (caseSuivante == CAISSE || caseSuivante == CAISSECIBLE){
        if (nx2 < 0 || nx2 >= LIGNE || ny2 < 0 || ny2 >= COLONNE) return;
        
        char caseApres = plateau[nx2][ny2];
        if (!(caseApres == AUTRE || caseApres == CIBLE)) return;

        if (caseApres == CIBLE) plateau[nx2][ny2] = CAISSECIBLE;
        else plateau[nx2][ny2] = CAISSE;

        if (caseSuivante == CAISSECIBLE) plateau[nx][ny] = CIBLE;
        else plateau[nx][ny] = AUTRE;

        if (plateau[sokobanX][sokobanY] == SOKOCIBLE) plateau[sokobanX][sokobanY] = CIBLE;
        else plateau[sokobanX][sokobanY] = AUTRE;

        if (caseSuivante == CAISSECIBLE) plateau[nx][ny] = SOKOCIBLE;
        else plateau[nx][ny] = SOKOBAN;

        /* Mémorisation V2 : 1 = Poussée */
        memoriser_deplacements(touche, historique, nbdp, 1);
        return;
    }

    /* Cas Simple */
    if (caseSuivante == AUTRE || caseSuivante == CIBLE){
        if (plateau[sokobanX][sokobanY] == SOKOCIBLE) plateau[sokobanX][sokobanY] = CIBLE;
        else plateau[sokobanX][sokobanY] = AUTRE;

        if (caseSuivante == CIBLE) plateau[nx][ny] = SOKOCIBLE;
        else plateau[nx][ny] = SOKOBAN;

        /* Mémorisation V2 : 0 = Simple */
        memoriser_deplacements(touche, historique, nbdp, 0);
        return;
    }
}

void initialiser_cibles(t_plateau plateau){
    for (int i = 0; i < LIGNE; i++){
        for (int j = 0; j < COLONNE; j++){
            cibles[i][j] = false;
            if (plateau[i][j] == CIBLE ||
                plateau[i][j] == CAISSECIBLE ||
                plateau[i][j] == SOKOCIBLE){
                cibles[i][j] = true;
            }
        }
    }
}

bool gagne(t_plateau plateau){
    for (int i = 0; i < LIGNE; i++){
        for (int j = 0; j < COLONNE; j++){
            if (cibles[i][j]){
                if (plateau[i][j] != CAISSECIBLE) return false;
            }
        }
    }
    return true;
}

/**
 * @brief Enregistre un coup dans l'historique (V2).
 * @param pousseCaisse 1 si une caisse a été poussée (majuscule), 0 sinon (minuscule).
 */
void memoriser_deplacements(char touche, t_tabDeplacement historique, int *nb, int pousseCaisse){
    if (*nb >= MAX_DEPLACEMENTS) return;

    char code = 0;

    if (pousseCaisse){
        switch (touche){
            case GAUCHE : code = SOKOGAUCHE; break;
            case AVANCER : code = SOKOHAUT; break; 
            case RECULER : code = SOKOBAS; break; 
            case DROITE : code = SOKODROITE; break;
        }
    } else {
        switch (touche){
            case GAUCHE : code = SGAUCHE; break;
            case AVANCER : code = SHAUT; break;
            case RECULER : code = SBAS; break;
            case DROITE : code = SDROITE; break;
        }
    }

    if (code != 0){
        historique[*nb] = code;
        (*nb)++; 
    }
}

/**
 * @brief Sauvegarde le fichier de replay (V2).
 */
void enregistrer_deplacements(t_tabDeplacement historique, int nb, char fic[]){
    FILE * f;
    f = fopen(fic, "w");
    if (f != NULL){
        fwrite(historique, sizeof(char), nb, f);
        fclose(f);
    }
}

/**
 * @brief Fonction technique complexe : Annulation (V2).
 * @details Lit le dernier coup, calcule l'inverse, et tire la caisse si nécessaire.
 */
void annuler_deplacement(t_plateau plateau, t_tabDeplacement historique, int *nbdp){
    if (*nbdp <= 0) return;

    /* 1. Lecture et suppression du dernier coup */
    (*nbdp)--; 
    char dernierCoup = historique[*nbdp];
    historique[*nbdp] = 0; 

    /* 2. Calcul du vecteur inverse */
    int directionInverseX = 0; 
    int directionInverseY = 0;

    switch(dernierCoup){
        case SHAUT : case SOKOHAUT : directionInverseX = 1;  break;
        case SBAS : case SOKOBAS : directionInverseX = -1; break;
        case SGAUCHE : case SOKOGAUCHE : directionInverseY = 1;  break; 
        case SDROITE : case SOKODROITE: directionInverseY = -1; break; 
    }

    /* 3. Coordonnées */
    int currentX = sokobanX;
    int currentY = sokobanY;
    int prevX = sokobanX + directionInverseX; 
    int prevY = sokobanY + directionInverseY;

    /* 4. Si c'était une poussée, on doit RAMENER la caisse */
    if (dernierCoup == SOKOHAUT || dernierCoup == SOKOBAS || dernierCoup == SOKOGAUCHE || dernierCoup == SOKODROITE){
        /* La caisse est actuellement "devant" (dans le sens du mouvement original) */
        int caisseX = sokobanX - directionInverseX; 
        int caisseY = sokobanY - directionInverseY;

        /* Effacer la caisse de là-bas */
        if (cibles[caisseX][caisseY]) plateau[caisseX][caisseY] = CIBLE;
        else plateau[caisseX][caisseY] = AUTRE;

        /* La remettre sous les pieds de Sokoban (il la tire) */
        if (cibles[currentX][currentY]) plateau[currentX][currentY] = CAISSECIBLE;
        else plateau[currentX][currentY] = CAISSE;
    } 
    else {
        /* Déplacement simple : juste vider la case */
        if (cibles[currentX][currentY]) plateau[currentX][currentY] = CIBLE;
        else plateau[currentX][currentY] = AUTRE;
    }

    /* 5. Remettre Sokoban à sa place précédente */
    if (cibles[prevX][prevY]) plateau[prevX][prevY] = SOKOCIBLE;
    else plateau[prevX][prevY] = SOKOBAN;
    
    /* 6. Mettre à jour les globales */
    sokobanX = prevX;
    sokobanY = prevY;
}


/* --- FONCTIONS I/O FOURNIES --- */

void charger_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrer_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne='\n';

    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

int kbhit(){
    int unCaractere=0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if(ch != EOF){
        ungetc(ch, stdin);
        unCaractere=1;
    } 
    return unCaractere;
}
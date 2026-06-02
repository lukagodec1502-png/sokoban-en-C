#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAILLE 12
#define MAX_DEP 500
#define MAX_BOX 50

#define PLAYER '@'
#define TARGET '.'
#define PLAYERTARGET '+'
#define BOX '$'
#define BOXTARGET '*'
#define WALL '#'
#define AIR ' '

typedef struct {
    int x, y;
} Position;
typedef char Plateau[TAILLE][TAILLE];

typedef struct {
    Position pos[MAX_BOX + 1];
    int nbBox;
} Etat;

void chargerPlateau(Plateau p, char *nom) {
    FILE *f = fopen(nom, "r");
    if (!f) {
        printf("Erreur lecture .sok\n");
        exit(1);
    }
    for (int y = 0; y < TAILLE; y++) {
        for (int x = 0; x < TAILLE; x++)
            fread(&p[y][x], 1, 1, f);
        fgetc(f);
    }
    fclose(f);
}

int chargerDeps(char *d, char *nom) {
    FILE *f = fopen(nom, "r");
    if (!f)
        return 0;
    int n = 0, c;
    while ((c = fgetc(f)) != EOF && n < MAX_DEP) {
        if (strchr("zsqdZSQDu", c))
            d[n++] = (char)c;
    }
    fclose(f);
    return n;
}

void afficher(Plateau p, int coup, int total) {
    system("clear");
    printf("Coup %d / %d\n\n", coup, total);
    for (int y = 0; y < TAILLE; y++) {
        for (int x = 0; x < TAILLE; x++)
            printf("%c", p[y][x]);
        printf("\n");
    }
}

Etat calculerEtat(Plateau p) {
    Etat e;
    e.nbBox = 0;
    for (int y = 0; y < TAILLE; y++) {
        for (int x = 0; x < TAILLE; x++) {
            if (p[y][x] == PLAYER || p[y][x] == PLAYERTARGET)
                e.pos[0] = (Position){x, y};
            else if (p[y][x] == BOX || p[y][x] == BOXTARGET) {
                if (e.nbBox < MAX_BOX)
                    e.pos[++e.nbBox] = (Position){x, y};
            }
        }
    }
    return e;
}

void restaurerPlateau(Plateau p, Etat e) {
    for (int y = 0; y < TAILLE; y++) {
        for (int x = 0; x < TAILLE; x++) {
            if (p[y][x] != WALL) {
                if (p[y][x] == TARGET || p[y][x] == BOXTARGET || p[y][x] == PLAYERTARGET)
                    p[y][x] = TARGET;
                else
                    p[y][x] = AIR;
            }
        }
    }
    int px = e.pos[0].x, py = e.pos[0].y;
    p[py][px] = (p[py][px] == TARGET) ? PLAYERTARGET : PLAYER;
    for (int i = 1; i <= e.nbBox; i++) {
        int bx = e.pos[i].x, by = e.pos[i].y;
        p[by][bx] = (p[by][bx] == TARGET) ? BOXTARGET : BOX;
    }
}

bool etatsIdentiques(Etat a, Etat b) {
    if (a.nbBox != b.nbBox)
        return false;
    for (int i = 0; i <= a.nbBox; i++)
        if (a.pos[i].x != b.pos[i].x || a.pos[i].y != b.pos[i].y)
            return false;
    return true;
}

bool tryMove(Plateau p, char dir) {
    int dx = 0, dy = 0, x = -1, y = -1;
    if (dir == 'z' || dir == 'Z')
        dy = -1;
    else if (dir == 's' || dir == 'S')
        dy = 1;
    else if (dir == 'q' || dir == 'Q')
        dx = -1;
    else if (dir == 'd' || dir == 'D')
        dx = 1;
    else
        return false;

    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (p[i][j] == PLAYER || p[i][j] == PLAYERTARGET) {
                y = i;
                x = j;
            }
        }
    }
    if (x == -1)
        return false;

    int nx = x + dx, ny = y + dy;
    int nnx = x + 2 * dx, nny = y + 2 * dy;

    if (ny < 0 || ny >= TAILLE || nx < 0 || nx >= TAILLE)
        return false;
    if (p[ny][nx] == WALL)
        return false;

    if (p[ny][nx] == BOX || p[ny][nx] == BOXTARGET) {
        if (nny < 0 || nny >= TAILLE || nnx < 0 || nnx >= TAILLE)
            return false;
        if (p[nny][nnx] == WALL || p[nny][nnx] == BOX || p[nny][nnx] == BOXTARGET)
            return false;
        p[nny][nnx] = (p[nny][nnx] == TARGET) ? BOXTARGET : BOX;
        p[ny][nx] = (p[ny][nx] == BOXTARGET) ? TARGET : AIR;
    }

    p[y][x] = (p[y][x] == PLAYERTARGET) ? TARGET : AIR;
    p[ny][nx] = (p[ny][nx] == TARGET) ? PLAYERTARGET : PLAYER;
    return true;
}

bool estGagne(Plateau p) {
    for (int y = 0; y < TAILLE; y++)
        for (int x = 0; x < TAILLE; x++)
            if (p[y][x] == BOX)
                return false;
    return true;
}

int main() {
    Plateau p;
    char d_init[MAX_DEP], d_opti[MAX_DEP];
    Etat historique[MAX_DEP + 1];
    char f_sok[32], f_dep[32];

    printf("Fichier .sok : ");
    if (scanf("%31s", f_sok) != 1)
        return 1;
    printf("Fichier .dep : ");
    if (scanf("%31s", f_dep) != 1)
        return 1;

    chargerPlateau(p, f_sok);
    int nb_init = chargerDeps(d_init, f_dep);

    int opt_len = 0;
    historique[0] = calculerEtat(p);

    afficher(p, 0, nb_init);
    usleep(300000);

    for (int i = 0; i < nb_init; i++) {
        if (d_init[i] == 'u') {
            if (opt_len > 0) {
                opt_len--;
                restaurerPlateau(p, historique[opt_len]);
            }
        } else if (tryMove(p, d_init[i])) {
            Etat actuel = calculerEtat(p);
            int boucle = -1;
            for (int k = 0; k <= opt_len; k++) {
                if (etatsIdentiques(historique[k], actuel)) {
                    boucle = k;
                    break;
                }
            }

            if (boucle != -1) {
                opt_len = boucle;
                restaurerPlateau(p, historique[opt_len]);
            } else {
                d_opti[opt_len] = d_init[i];
                opt_len++;
                historique[opt_len] = actuel;
            }
        }

        afficher(p, i + 1, nb_init);
        usleep(300000);

        if (estGagne(p)) {
            printf("\n%d\n", opt_len);
            FILE *f = fopen("opti.dep", "w");
            if (f) {
                fwrite(d_opti, 1, opt_len, f);
                fclose(f);
            }
            return 0;
        }
    }

    if (estGagne(p))
        printf("\n%d\n", opt_len);
    else
        printf("\nD\n");

    return 0;
}
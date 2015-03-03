#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PLAYER_BLACK    0
#define PLAYER_WHITE    1

/*
 * Structure du jeu.
 * @board représente le plateau de jeu. Chaque case est représentée par
 *         une valeur entière dont la signification est la suivante:
 *         [...|C|T|P] sont les 3 bits de poid faible de chaque valeur
 *         entière. Le bit C est le bit de couleur: 0 = noir, 1 = blanc.
 *         Le bit T est le bit de type de pièce: 0 = pion, 1 = dame.
 *         Le bit P est le bit de présence: 0 = case vide, 1 = case remplie.
 *         board[i][j] représente donc l'état de la case aux coordonnées (i,j).
 * @xsize et @ysize stockent la taille du jeu
 * @moves enregistre la liste des mouvements effectués
 * @cur_player enregistre le joueur actuel (i.e. qui doit effectuer le prochain mouvement)
 *             (PLAYER_BLACK ou PLAYER_WHITE)
 */
struct game{
    int **board;
    int xsize, ysize;

    struct move *moves;
    int cur_player;
};

/* Coordonnées */
struct coord {
    int x, y;
};

/*
 * Liste de la séquence d'un mouvement
 */
struct move_seq {
    struct move_seq *next;
    struct coord c_old;     /* coordonnées de départ */
    struct coord c_new;     /* coordonnées de destination */

    /*
     * Les champs suivants ne sont utilisés que lorsque l'élément fait partie
     * des mouvements effectués dans game->moves.
     *
     * @piece_value est la valeur entière d'une pièce capturée lors du mouvement.
     *              0 si pas de pièce capturée.
     * @piece_taken contient les coordonnées de la pièce capturée
     * @old_orig contient la valeur entière de la pièce se situant en @c_old avant
     *           l'exécution du mouvement.
     */
    int piece_value;
    struct coord piece_taken;
    int old_orig;
};

/*
 * Liste de mouvements
 */
struct move {
    struct move *next;
    struct move_seq *seq;
};

/*
 * new_game
 * Créer un nouveau jeu avec la position initiale
 * ysize >= 3 et xsize >=2
 *
 * @xsize: taille du jeu pour les abscisses (i.e. nombre de colonnes)
 * @ysize: taille du jeu pour les ordonnées (i.e. nombre de lignes)
 * @return: Pointeur vers la structure du jeu ou NULL si erreur
 */
struct game *new_game(int xsize, int ysize){
	struct game *g;
	g = (struct game *) malloc(sizeof(struct game));
	if (g==NULL) {return(NULL);}
	g->xsize=xsize;
	g->ysize=ysize;
	g->board = (int**) malloc(sizeof(int)*xsize*ysize);
	if (g->board==NULL) {return(NULL);}
	int i,j;
	for (i=0;i<xsize;i++) {
		g->board[i]=(int*) malloc(sizeof(int)*ysize);
		if(g->board[i]==NULL){return(NULL);}
		for (j=0;j<ysize;j++){
			if(j<((ysize-2+ysize%2)/2)){
				if((i+j)%2!=0){				
					g->board[i][j]=0b00000001;
				}
				else{
					g->board[i][j]=0b00000000;
				}
			}
			else if(j>((ysize-ysize%2)/2)){
				if((i+j)%2!=0){
					g->board[i][j]=0b00000101;
				}
				else{
					g->board[i][j]=0b00000000;
				}
			}
			else{
				g->board[i][j]=0b00000000;
			}
		}
	}
	g->cur_player=PLAYER_WHITE;
	g->moves=NULL;
	return g;	
}

/*
 * free_game
 * Libérer les ressources allouées à un jeu
 *
 * @game: pointeur vers la structure du jeu
 */
void free_game(struct game *game){
	int i;
	for(i=0;i<game->xsize;i++){
		free(game->board[i]);
		game->board[i]=NULL;
	}
	free(game->board);
	game->board=NULL;
	free(game);
	game=NULL;
}


/*
 * is_move_seq_valid
 * Vérifier si une séquence d'un mouvement est valide. La fonction ne MODIFIE PAS l'état du jeu !
 * 
 * @game: pointeur vers la structure du jeu
 * @seq: séquence du mouvement à vérifier
 * @prev: séquence précédent immédiatement la séquence à vérifier, NULL si @seq est la première séquence du mouvement
 * @taken: pointeur vers des coordonnées qui seront définies aux coordonnées de la pièce prise s'il en existe une
 * @return: 0 si mouvement invalide, 1 si déplacement uniquement, 2 si capture
 */
int is_move_seq_valid(const struct game *game, const struct move_seq *seq, const struct move_seq *prev, struct coord *taken){
	
	//1 : Le pion reste sur le plateau
	if (seq->c_new.x >= game->xsize || seq->c_new.y >= game->ysize || seq->c_new.x < 0 || seq->c_new.y < 0){
		return (0);
	}

	//2 : Mouvement adjoint au précédent
	else if (prev != NULL){
		if (prev->c_new.x != seq->c_old.x || prev->c_new.y != seq->c_old.y){
			return (0);
		}
	}

	//3 : Le joueur joue ses propres pions
	else if (game->cur_player != seq->old_orig>>2){
		return (0);
	}
	
	//4 : La case d'arrivée est libre
	else if (game->board[seq->c_new.x][seq->c_new.y] != 0){
		return (0);
	}

	//5 : Le mouvement est en diagonale
	else if (fabs(seq->c_new.x - seq->c_old.x) != fabs(seq->c_new.y - seq->c_old.y){
		return (0);
	}

	//PION
	if ((seq->old_orig>>1)%2 == 0){
		//1 : Prise & le pion capturé est de la bonne couleur & en avant
		if (fabs(seq->c_new.x - seq->c_old.x)==2.0 && 
		    game->board[seq->c_old.x+(seq->c_new.x - seq->c_old.x)/2][seq->c_old.y+(seq->c_new.y - seq->c_old.y)/2]>>2 != seq->old_orig>>2 && 
		    ((seq->c_new.y - seq->c_old.y) < 0) == (seq->old_orig>>2)){
			taken.x = seq->c_old.x+(seq->c_new.x - seq->c_old.x)/2;
			taken.y = seq->c_old.y+(seq->c_new.y - seq->c_old.y)/2
			return (2);
		}
		//2 : Déplacement de 1 en avant
		else if (fabs(seq->c_new.x - seq->c_old.x)==1.0 && ((seq->c_new.y - seq->c_old.y) < 0) == (seq->old_orig>>2)){
			return (1);
		}
		else {
			return (0);
		}
	}

	//Dame
	else {
		//
		int i=0;
		while(
	}
	
	return (1);
}


/*
 * print_board
 * Affiche l'état du jeu sur le terminal
 *
 * @game: pointeur vers la structure du jeu
 */
void print_board(const struct game *game){
	int i,j;
	printf("\n");
	for(j=0;j<game->ysize;j++){
		for(i=0;i<game->xsize;i++){
			printf("* * * * ");
		}
		printf("*\n");
		for(i=0;i<game->xsize;i++){
			printf("*       ");
		}
		printf("*\n");
		for(i=0;i<game->xsize;i++){
			printf("*   %d   ",game->board[i][j]);
		}
		printf("*\n");
		for(i=0;i<game->xsize;i++){
			printf("*       ");
		}
		printf("*\n");
	}
	for(i=0;i<game->xsize;i++){
		printf("* * * * ");
	}
	printf("*\n \n");
}


/*
 * main : fonction provisoire : programme.c contiendra main
 *
 */
int main(int argc, char* argv[]){
	struct game* game = new_game(10,10);

	print_board(game);
	free_game(game);

	return(EXIT_SUCCESS);
}

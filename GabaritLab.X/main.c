 /**
 * @file main.c  
 * @author 
 * @date   
 * @brief  
 *
 * @version 1.0
 * Environnement:
 *     Développement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.00)
 *     Matériel: Carte démo du Pickit3. PIC 18F45K20
  */

/****************** Liste des INCLUDES ****************************************/
#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#include <conio.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "Lcd4Lignes.h"


/********************** CONSTANTES *******************************************/
#define _XTAL_FREQ 1000000 //Constante utilisée par __delay_ms(x). Doit = fréq interne du uC

#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 2 //caractère cgram d'une tuile
#define MINE 3 //caractère cgram d'une mine

/********************** PROTOTYPES *******************************************/
void initialisation(void);
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void deplace(char* x, char* y);
bool demine(char x, char y);
void enleveTuilesAutour(char x, char y);
bool gagne(int* pMines);
void afficheTabVue(void);
void afficheTabMine(void);
char getAnalog(char canal);

/****************** VARIABLES GLOBALES ****************************************/
 char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau des caractères affichés au LCD
 char m_tabMines[NB_LIGNE][NB_COL+1];//Tableau contenant les mines, les espaces et les chiffres

/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{
    char x = 1;
    char y = 1;
    int nbMines = 4;
    
    initialisation();
    lcd_init();//initialisation de l'affichage LCD
    //lcd_cacheCurseur();//cache le curseur pour une meilleure expérience visuelle
    lcd_effaceAffichage();//efface l'affichage pour faire place au nouveau message
    lcd_curseurHome();//met le curseur à zéro
    
    initTabVue();
    rempliMines(nbMines);
    metToucheCombien();
    afficheTabVue();
    
    while(1) //boucle principale
    {
        deplace(&x, &y);
        if(PORTBbits.RB1 == 0){
            if(!demine(x, y) || gagne(&nbMines)){
                afficheTabMine();
                while(!PORTBbits.RB1 == 0);
                initTabVue();
                rempliMines(nbMines);
                metToucheCombien();
                afficheTabVue();
                lcd_gotoXY(x, y);
            }
        }
        __delay_ms(100);
    }
}

void afficheTabVue(void){
    lcd_effaceAffichage();
    for(int i = 0; i < NB_LIGNE; i++){
        lcd_gotoXY(1, i+1);
        lcd_putMessage(m_tabVue[i]);
    }
}

void afficheTabMine(void){
    lcd_effaceAffichage();
    for(int i = 0; i < NB_LIGNE; i++){
        lcd_gotoXY(1, i+1);
        lcd_putMessage(m_tabMines[i]);
    }
}

/*
 * @brief Rempli le tableau m_tabVue avec le caractère spécial (définie en CGRAM
 *  du LCD) TUILE. Met un '\0' à la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void){
    for(int i = 0; i<NB_LIGNE; i++){//boucle pour les lignes
        for(int j = 0; j<NB_COL; j++){//boucle pour les colonnes
            m_tabVue[i][j] = TUILE;//remplace le caractere dans la ligne et la colonne spécifiéepar un espace
        }
        m_tabVue[i][NB_COL] = '\0';
    }
}
 
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caractère MINE défini en CGRAM.
 * @param int nb, le nombre de mines à mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb){
    char col = 0;
    char ligne = 0;
    
    for(int i = 0; i < NB_COL; i++){
        for(int j = 0; j < NB_LIGNE; j++){
            m_tabMines[j][i] = 32;
        }
    }
    
    for(int i = 0; i < nb; i++){
        do{
           col = rand()%NB_COL;
           ligne = rand()%NB_LIGNE;
        }while(m_tabMines[ligne][col] != 32);
        m_tabMines[ligne][col] = MINE;
    }
}
 
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche à 3 mines, alors la méthode place le code ascii de 3 dans
 * le tableau. Si la case ne touche à aucune mine, la méthode met le code
 * ascii d'un espace.
 * Cette méthode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void){
    for(int i = 0; i<NB_COL; i++){//boucle pour les lignes
        for(int j = 0; j<NB_LIGNE; j++){//boucle pour les colonnes
            if(m_tabMines[j][i] != MINE){
                m_tabMines[j][i] = calculToucheCombien(j, i);
            }
        }
    }
}
 
/*
 * @brief Calcul à combien de mines touche la case. Cette méthode est appelée par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a vérifier
 * @return char nombre. Le nombre de mines touchées par la case
 */
char calculToucheCombien(int ligne, int colonne){
    int x = 0;
    int y = 0;
    char total = 0;

    for(int i = -1; i < 2; i++){
        for(int j = -1; j < 2; j++){
            if(j != 0 || i != 0){
                x = colonne + i;
                y = ligne + j;
                if(x >= 0 && x < NB_COL && y >= 0 && y < NB_LIGNE){
                    if(m_tabMines[y][x] == MINE){
                        total++;
                    }
                }
            }
        }
    }
    if(total == 0){
        return 32;
    }
    return total+48;
}
 
/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* px, char* py){
    int aX = getAnalog(AXE_X);
    int aY = getAnalog(AXE_Y);
    
    if(aX < 100){//reste à créer les limites et la "téléportation" d'un coté à l'autre.
        (*px)--;
        if(*px <= 0){
            *px = NB_COL;
        }
        lcd_gotoXY((*px), (*py));
    }else if(aX > 150){
        (*px)++;
        if(*px > NB_COL){
            *px = 1;
        }
        lcd_gotoXY((*px), (*py));
    }else if(aY < 100){
        (*py)++;
        if(*py > NB_LIGNE){
            *py = 1;
        }
        lcd_gotoXY((*px), (*py));
    }else if(aY > 150){
        (*py)--;
        if(*py <= 0){
            *py = NB_LIGNE;
            
        }
        lcd_gotoXY((*px), (*py));
    }
    
    
}
 
/*
 * @brief Dévoile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derrière les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y){
    
    while(PORTBbits.RB1 == 0);
    
    x--;
    y--;
    
    if(m_tabMines[y][x] == MINE){
        lcd_gotoXY(x+1, y+1);
        return false;
    }
    if(m_tabMines[y][x] == 32){
        enleveTuilesAutour(x, y);
    }else if(m_tabMines[y][x] >= 48){
        m_tabVue[y][x] = m_tabMines[y][x];
    }
    afficheTabVue();
    lcd_gotoXY(x+1, y+1);
    return true;
}
 
/*
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y){
    
    char colonne = 0;
    char ligne = 0;
    
    for(int i = -1; i < 2; i++){
            for(int j = -1; j < 2; j++){
                colonne = x + i;
                ligne = y + j;
                m_tabVue[ligne][colonne] = m_tabMines[ligne][colonne];
            }
        }
}
 
/*
 * @brief Vérifie si gagné. On a gagné quand le nombre de tuiles non dévoilées
 * est égal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagné.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagné, faux sinon
 */
bool gagne(int* pMines){
    char ttl = 0;
    for(int i = 0; i<NB_LIGNE; i++){//boucle pour les lignes
        for(int j = 0; j<NB_COL; j++){//boucle pour les colonnes
            if(m_tabVue[i][j] == TUILE){
                ttl++;
            }
        }
    }
    if(ttl == (*pMines)){
        (*pMines)++;
        return true;
    }
    return false;
}

/*
 * @brief Lit le port analogique. 
 * @param Le no du port à lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */
char getAnalog(char canal)
{ 
    ADCON0bits.CHS = canal;
    __delay_us(1);  
    ADCON0bits.GO_DONE = 1;  //lance une conversion
    while (ADCON0bits.GO_DONE == 1) //attend fin de la conversion
        ;
    return  ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}

/**
 * @brief Fait l'initialisation des différents regesitres du PIC
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{  
    
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 à RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN à on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement à gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB à gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fréquence pour la conversion la plus longue possible)
 
 
    
}


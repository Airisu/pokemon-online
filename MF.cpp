#include "MF.hh"
#include "SDL_gfx.h"

using namespace std;

/* Change hauteur et largeur */
void MF_Base::resize(Uint16 w, Uint16 h)
{
    dims.w = w;
    dims.h = h;
    set_updated();

    if (pSup == NULL) return;
}

/* Bouge la fenetres */
void MF_Base::move(Sint16 x, Sint16 y)
{
    if (dims.x == x && dims.y == y) return;
    //SDL fait que l'affichage merdouille avec des vals n�gatives
    dims.x = MAX(x, 0);
    dims.y = MAX(y, 0);
    set_updated();
}

/* change de mani�re brute le rectangle de dimensions, sans
   aucun autre controle */
void MF_Base::setRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    set_updated();
    dims.x = x;
    dims.y = y;
    dims.w = w;
    dims.h = h;
}

/* Change la couleur de fond */
void MF_Base::setColor(const Color &c)
{
    set_updated();
    bgColor = c;
}

/** Classe Sup!! **/

/* utilis�e internallement */
/* On recherche une ID de libre
   � donner.. */
Uint16 MF_Boss::getfreeID() const
{
    /* cr�ation d'un tableau pour dire si les ID sont prises ou pas */
    bool IDtable[nbMF];

    /* On remplit le tableau en parcourant tout */
    for (MF_Base *tmp = pDebut; tmp != NULL; tmp = tmp->pApres)
    {
        if (tmp->ID < nbMF)
            IDtable[tmp->ID] = true;
    }

    /* Maintenant on cherche la premi�re libre */

    for (Uint16 i=0; i < nbMF; ++i)
    {
        if (IDtable[i] == false)
            return i;
    }

    /* Si la recherche n'a pas �t� fructueuse, toutes les cases sont
       remplies; il y a autant de cases que de fen�tres. Donc celle
       d'apr�s est forc�ment vide. */
    return nbMF;
}

/* gestion de fenetres */
/* on donne un place et met en premier une nouvelle fenetre donn�e */
MF_Base * MF_Boss::allouer(MF_Base *fenetre)
{
    if (fenetre->pSup != false)
    {
        return NULL;
    }

    /* On augmente le nombre de fenetres (+1) */
    ++nbMF;

    /* On r�cup�re une ID libre*/
    fenetre->ID = getfreeID();

    /* et on l'int�gre en premi�re position de la liste chain�e */
    if (pDebut != NULL)
    {
        pDebut->pAvant = fenetre;
        pDebut->setPos(false);
    }
    fenetre->pApres = pDebut;
    pDebut = fenetre;
    fenetre->setPos(true);
    if (pFin == NULL)
        pFin = fenetre;
    fenetre->pAvant = NULL;
    fenetre->pSup = this;

    set_updated();

    return fenetre;
}

/* d�salloue en sorte que lors de la destruction
   de fenetre celles d'apr�s ne soient pas entrain�e
   par la liste chain�e */
MF_Base * MF_Boss::desallouer(MF_Base *fenetre)
{
    //Si c'est pas la notre, ca peut entrainer des bugs, difficiles � localiser
    if (fenetre->pSup != this)
        return fenetre;
    /* r�duction du nombre de fen�tres */
    --nbMF;

    /* reconstruction de la cha�ne */
    if (pDebut == fenetre)
    {
        pDebut = fenetre->pApres;
        if (pDebut != NULL)
            pDebut->setPos(true);
    }
    else if (fenetre->pAvant != NULL)
        fenetre->pAvant->pApres = fenetre->pApres;
    if (pFin == fenetre)
        pFin = fenetre->pAvant;
    else if (fenetre->pApres != NULL)
        fenetre->pApres->pAvant = fenetre->pAvant;

    /* oubli des autres membres d'avant et d'apr�s de la fenetre */
    fenetre->pAvant = NULL;
    fenetre->pApres = NULL;
    fenetre->pSup = NULL;

    set_updated();

    return fenetre;
}

/* On l'arrache de sa position et on la met en premi�re */
void MF_Boss::polepos(MF_Base *fenetre)
{
    /* si d�j� prems */
    if (pDebut == fenetre)
        return;

    /* arrachement... */
    if (fenetre->pApres != NULL){
        fenetre->pApres->pAvant = fenetre->pAvant;
    }

    if (fenetre->pAvant != NULL)
    {
        fenetre->pAvant->pApres = fenetre->pApres;
        if (fenetre == pFin)
            pFin = fenetre->pAvant;
    }

    /* puis replacement */
    fenetre->pAvant = NULL;
    fenetre->pApres = pDebut;
    pDebut->setPos(false);
    fenetre->pApres->pAvant = fenetre;
    pDebut = fenetre;
    pDebut->setPos(true);
}
/* Avoir le n-i�me membre */
MF_Base* MF_Boss::get_member(int index)
{
    MF_Base *it = pDebut;

    while (index > 0 && it->pApres != NULL)
    {
        it = it->pApres;
        index --;
    }

    return it;
}

/* idem */
void MF_Boss::move(Sint16 x, Sint16 y)
{
    if (x == dims.x && y == dims.y) return;

    //MF_Base ne va pas forc�ment aux x et y indiqu�s, donc je sauvegarde les
    //ancienne pos, au lieu de faire ce que j'ai � faire et d'ex�cuter MF_Base
    //avant
    Sint16 old_x = dims.x;
    Sint16 old_y = dims.y;

    MF_Base::move(x, y);
    for (MF_Base *tmp = pDebut; tmp != NULL; tmp=tmp->pApres)
    {
        tmp->move(dims.x-old_x+tmp->dims.x, dims.y-old_y+tmp->dims.y);
    }

    updated = false;
}

/* affichage */
void MF_Boss::affiche(Surface &surface)
{
    updated = true;

    /* On affiche d'abord soi-m�me */
    MF_Base::affiche(surface);

    /* Puis les petites fen�tres */
    afficheMF(surface);
}

/* affiche seulement les sous-fen�tres
   ainsi lors de l'h�ritage cette fonction sera constante,
   ce qui �vite de coller le morceau de code dans la fonction
   affiche � chaque fois en faisant un simple appel de fonction. */
int MF_Boss::afficheMF(Surface &surface)
{
    /* boucle classique */
    /* Apres si vous voulez vous pouvez vous compliquer la
       vie pour n'afficher que ce qui est n�cessaire (pas
       ce qui est cach�) mais zzzzzzzzzzzzzzzzzzzzzzzz   */
    for (MF_Base *tmp = pFin; tmp!=NULL; tmp=tmp->pAvant)
    {
        tmp->affiche(surface);
    }
    return 0;
}

/* gestion d'�v�nements */
bool MF_Boss::gereEvenement(const SDL_Event &event)
{
    /* Si pas de fen�tres */
    if (pDebut == NULL)
    {
        return evntPerso(event);
    }

    bool changed = false;

    switch(event.type)
    {
        /* si motion ou touche, c'est la premi�re fen�tre qui en b�n�ficie */
        case SDL_MOUSEMOTION:
        //on fait le hover
        {
            bool finished = false;
            for (MF_Base *tmp = pDebut; tmp != NULL; tmp = tmp->pApres)
            {
                if (!finished && tmp->dedans(event.motion.x, event.motion.y))
                {
                    changed |= tmp->set_hover_state(true);
                    finished = true;
                } else
                {
                    changed |= tmp->set_hover_state(false);
                }
            }
        }
        case SDL_MOUSEBUTTONUP:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return pDebut->gereEvenement(event) | changed;
        /* Pour le clic on cherche dans quelle fen�tre on a cliqu�! */
        case SDL_MOUSEBUTTONDOWN:
        if (event.button.button != SDL_BUTTON_LEFT && event.button.button != SDL_BUTTON_RIGHT && event.button.button != SDL_BUTTON_MIDDLE) return pDebut->gereEvenement(event);
        for (MF_Base *tmp = pDebut; tmp != NULL; tmp = tmp->pApres)
        {
            /* si dedans, on place la fenetre en pos 1 */
            if (tmp->dedans(event.button.x, event.button.y))
            {
                if (tmp == pDebut)
                    return tmp->gereEvenement(event);
                else
                {
                    polepos(tmp);
                    tmp->gereEvenement(event);
                    return true;
                }
            }
        }
        /* Si aucune fenetre */
        changed = evntPerso(event);
        return pDebut->gereEvenement(event) | changed;
        default:
        return false;
    }
}

void MF_Directions::setDirections(MF_Base *tab, MF_Base *up, MF_Base *down, MF_Base *left, MF_Base *right)
{
    this->up = up;
    this->tab = tab;
    this->left = left;
    this->right = right;
    this->down = down;
}

bool MF_Directions::gereTouche(const SDL_KeyboardEvent &event)
{
    switch (event.keysym.sym)
    {
        case SDLK_TAB:
            envoieMessage("tab");
            return 1;
        case SDLK_LEFT:
            envoieMessage("left");
            return 1;
        case SDLK_RIGHT:
            envoieMessage("right");
            return 1;
        case SDLK_UP:
            envoieMessage("up");
            return 1;
        case SDLK_DOWN:
            envoieMessage("down");
            return 1;
        case SDLK_RETURN:
            envoieMessage("enter");
            return 1;
        default:
            return 0;
    }
}

bool MF_BDirections::gereDirection(const char* message, MF_Base *fenetre)
{
    MF_Directions *fen = dynamic_cast<MF_Directions *>(fenetre);
    if (fen == NULL) return false; //on g�re que les directions, pas les fen�tres normales
    //les messages right, left, down et up et tab permettent de changer de fen�tre
    if (strcmp(message, "right") == 0)
        if (fen->right == NULL) return false;
        else {
            polepos(fen->right);
            return true;
        }
    if (strcmp(message, "left") == 0)
        if (fen->left == NULL) return false;
        else {
            polepos(fen->left);
            return true;
        }
    if (strcmp(message, "up") == 0)
        if (fen->up == NULL) return false;
        else {
            polepos(fen->up);
            return true;
        }
    if (strcmp(message, "down") == 0)
        if (fen->down == NULL) return false;
        else {
            polepos(fen->down);
            return true;
        }
    if (strcmp(message, "tab") == 0)
        if (fen->tab == NULL) return false;
        else {
            polepos(fen->tab);
            return true;
        }
    //la commande ne concerne pas les directions
    return false;
}

MF_Surf::~MF_Surf()
{
}

void MF_Surf::affiche(Surface &surface)
{
    this->surface.blitTo(surface, 0, dims);
}

void MF_Surf::setRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    dims.x = x;
    dims.y = y;
    if (w != dims.w || h != dims.h)
    {
        resize(w, h);
    } else
    {
        set_updated();
    }
}

void MF_Surf::resize(Uint16 w, Uint16 h)
{
    /* On fait le boulot normal */
    MF_Base::resize(w, h);

    /* cr�ation de la nouvelle surface*/
    surface.create(8,w,h,SDL_HWSURFACE);
    surface.adapt();

    surface.fill(0, bgColor);
}

bool MF_MoveAbleBoss::gereEvenement(const SDL_Event &event)
{
    //On recherche les evenements quand on est cliqu�,
    //ou les clics sur nous, ce qui nous retombera dessus
    //avec le gereEvenement
    clicOn &= BUTTON_LEFT_ON;
    if (!clicOn) return MF_Boss::gereEvenement(event);
    return evntPerso(event);
}

int MF_MoveAbleBoss::evntPerso(const SDL_Event &event)
{
    //Si c'est un clic
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && SDL_IsInRect(&dims, event.button.x, event.button.y))
    {
        pos_clic_x = event.button.x - dims.x;
        pos_clic_y = event.button.y - dims.y;
        clicOn = true;

        pDebut->gereEvenement(event);
        return true;
    }

    //Si c'est une motion
    if (event.type != SDL_MOUSEMOTION || !clicOn)
        return false;

    move(event.motion.x - pos_clic_x, event.motion.y - pos_clic_y);
    return true;
}

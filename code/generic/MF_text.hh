#ifndef MF_TEXT_HH
#define MF_TEXT_HH

#include "MF.hh"

#include "interfont.hpp"
#include <string>
#include <vector>
#include "SDL_gfx.h"
#include <SDL/SDL_image.h>

/* Dans la lign�es des Multi-fen�tres voici celles
   qui displaynt du texte - 1 ligne seulement */

class MF_Line : virtual public MF_Surf
{
    public:
        /* texte contenu*/
        std::string texte;

        bool updated;

        /* police */
        Font police;
        /* couleur du texte */
        Color textColor;

        MF_Line();
        ~MF_Line();

        /* Met une couleur de texte */
        void setTextColor(const Color &c = 0);

        /* Pour balancer du texte */
        virtual void write(const std::string &texte, int pos);
        virtual void write(const char* texte, int pos);
        virtual void write(char texte, int pos);
        virtual void write(const std::string &texte);
        virtual void write(const char* texte);
        virtual void write(char texte);

        /* pour savoir la pos en nombre de lettres d'une abscisse */
        virtual int  toLen(int w) const;

        /* Pour en erase */
        virtual void erase(int depart = 0);
        virtual void erase(int depart, int nombre);

        /* Pour displayr ^^ */
        virtual void display(Surface &surface);

        /* Pour refresh la surface en m�moire */
        virtual void refresh();

        /* overload */
        virtual bool check_updated (){ return updated;}
        virtual void set_updated()
        {
            if (pSup != NULL) pSup->updated = false;
            updated = false;
        }

        void setFont(const char *path, Uint16 size);
        void shareFont(Font &police);
};

//juste pour aider
struct xy
{
    int x;
    int y;

    xy(int x=0, int y=0);
};

bool operator > (xy &a, xy &b);
bool operator < (xy &a, xy &b);
bool operator == (xy &a, xy &b);

/* C'est du multiligne cette fois.
   on progresse hein ? avec une barre d�roulante en +*/

class MF_MLine : virtual public MF_Line
{
public:
    int capacity;
    unsigned int ncurrentligne;

    /* un vecteur pour donner les pos des lignes (d�but et fin) */
    std::vector<xy> poslignes;

    /* Maintenant barre d�roulante */
    Surface flechehaut;
    Surface flechebas;
    bool barreOn;

    /* pour savoir si on a cliqu� sur la barre */
    bool barre_clicOn;

    /* et la pos relative du clic en hauteur par rapport au haut de la fenetre */
    int  barreclic_y;

    /* timerID, quand on clique sur une fl�che ou la partie vide de la zone de la barre
       d�roulante */
    SDL_TimerID timer_ID;
    bool timer_wasinit;
    //pos clic
    Sint16 posx;
    Sint16 posy;

    /* constructeur et patati ...*/
    MF_MLine();
    ~MF_MLine();

    /* Les fonctions pour displayr */
    /* lors du test je les avais oubli�... :( */
    virtual void display(Surface &ecran) {MF_Line::display(ecran);}

    /* Redessine tout le texte */
    virtual void refresh();

    /* pour avoir le nombre de lignes */
    virtual unsigned int get_nblignes() const;

    /* largeur max du texte */
    virtual int maxwidth() const;

    /*pour se changer en cas de trop plein de lignes */
    virtual void update_capacity();

    /* pour changer la capacit� */
    virtual void setcapacity(int capacite);

    /* Pour ajouter une barre si trop de lignes ou au contraire l'enlever */
    virtual void update_barre();

    /* Pour avoir le num de caract�res jusqu'� une largeur particuli�re */
    virtual int toLen(int noLine, int w) const;

    /* r�cup�rer la pos d'un caract�re � partir de son offset */
    virtual xy get_abspos(int offset);
    /* r�cup�rer la pos pour �crire apr�s le dernier caract�re */
    virtual xy get_abspos() const;

    /* r�cuperer l'offset d'un caract�re � partir de sa pos */
    virtual int get_offset(int x, int y) const;//no de ligne + curseury
    virtual int get_offset(const xy &abspos, int y) const; //ligne + curseury
    virtual int get_offset(const xy &abspos) const;//fin de ligne
    virtual int get_offset(int x) const;//debut de ligne
    /* r�cup�rer l'offset pour �crire apr�s le dernier caract�re */
    virtual int get_offset() const;

    /* nouvelles fonctions pour �crire */
    virtual void write (const std::string &texte, unsigned int posx, int posy);
    virtual void write (const char *texte, unsigned int posx, int posy);
    virtual void write (char texte, unsigned int posx, int posy);
    virtual void write (const std::string &texte) {write(texte, get_nblignes()-1, poslignes[get_nblignes()-1].y);}
    virtual void write (char texte) {MF_Line::write(texte);}
    virtual void write (const char *texte) {MF_Line::write(texte);}

    /* pour adapter le contenu des lignes en fonction de la largeur maximale */
    virtual void update_largeur(unsigned int noligne, bool suivantes);

    /* gestion �v�nements pour faire defiler texte */
    virtual bool deal_w_Event(const SDL_Event &event);
    virtual bool deal_w_motion(const SDL_MouseMotionEvent &motionevent);
    virtual bool deal_w_key(const SDL_KeyboardEvent &keyboardevent);
    virtual bool deal_w_click(const SDL_MouseButtonEvent &buttonevent);

    /* faire monter la barre d'un cran */
    virtual bool montebarre(int crans);
    virtual bool descendbarre(int crans);

    /* pour s'aider */
    virtual int maxlignes() const {return dims.h/police.line_skip();}
    virtual int getbarreh() const {return dims.h/3;}
    virtual int getbarrey(int barreh) const;
    virtual void displaybarre(int barrey, int barreh);
    //A cause du timer
    virtual void resize(Uint16 w, Uint16 h)
    {
        if (timer_wasinit == true)
        {
            SDL_RemoveTimer(timer_ID);
            timer_wasinit = false;
        }
        MF_Surf::resize(w, h);
    }
    virtual void erase(int depart, int nombre=-1);
};

Uint32 MF_MLine_fonction_timer(Uint32 interval, void *param);

/* On peut faire plusieurs choses:
   write isIn avec un curseur,
   ou juste utiliser la souris pour selectionner du
   texte. Toujours uniligne */

class MF_WLine : virtual public MF_Line
{
    public:
        /*special*/
        bool second_update;
        /* Trucs normaux */
        int curseury; //position du curseur ^^
        bool ecrivable; //Si on peut �crire
        bool curseurOn; //Si on peut displayr le cuseur
        /* Maintenant pour le surlignage */
        bool clicOn;
        bool surligneOn; //Si highlight on
        int curseuryDeb; //Si highlight, on retient la pos de d�but
        bool curseurDisp; //etat du curseur
        Uint32 lastCursor; //Dernier affichage bien du curseur

        MF_WLine();
        ~MF_WLine();

        /* Gere clic, motion et touche */
        virtual bool deal_w_Event(const SDL_Event &event);
        virtual bool deal_w_key(const SDL_KeyboardEvent &keyevent);
        virtual bool deal_w_click(const SDL_MouseButtonEvent &buttonevent);
        virtual bool deal_w_motion(const SDL_MouseMotionEvent &motionevent);

        /* Pour write du texte */
        virtual void write(const std::string &texte, int pos);
        virtual void write(const char* texte, int pos);
        virtual void write(char texte, int pos);
        virtual void write(const std::string &texte) {write(texte, curseury);}
        virtual void write(const char* texte) {write(texte, curseury);}
        virtual void write(char texte) {write(texte, curseury);}

        /* erase l� ou c'est surlign� - retourne true si modification*/
        virtual bool clearHighlight();
        /* commencer si possible le higlight */
        virtual void startHighlight();
        /* voir si on doit enlever le highlight, ...  -- retourne true si modification*/
        virtual bool updateHighlight();

        /* displayr et refresh */
        virtual void display(Surface &ecran);
        virtual void refresh();
        /* pour changer le highlight */
        virtual void setPos(bool prems);
        virtual bool check_updated() { return updated && SDL_GetTicks()-lastCursor < 500 && second_update;}
};

/* c'est une combinaison du texte unilignes pour �crire (MF_uwtext) et du texte
   multilignes (MF_mtext).
   On peut donc �crire sur plusieurs lignes :) */

class MF_MWLine : virtual public MF_MLine, virtual public MF_WLine
{
public:
    /* il reste le curseurx � g�rer */
    unsigned int curseurx;
    unsigned int curseurxDeb;
    /* et c'est tout :) */

    /** Fonctions **/
    /*constructeur et destructeur*/
    MF_MWLine();
    ~MF_MWLine();

    /* display et actualise la surface en m�moire */
    virtual void display(Surface &ecran);
    virtual void refresh();

    /* nouvelles fonctions pour �crire */
    /* elles updatent aussi le curseur :) */
    virtual void write (const std::string &texte, unsigned int posx, unsigned int posy);
    virtual void write (const char *texte, unsigned int posx, unsigned int posy);
    virtual void write (char texte, unsigned int posx, unsigned int posy);
    virtual void write (const std::string &texte){write(texte, curseurx, curseury);}
    virtual void write (char texte){write(texte, curseurx, curseury);}
    virtual void write (const char *texte){write(texte, curseurx, curseury);}
    /* gestion �v�nements pour faire defiler texte et �crire */
    virtual bool deal_w_Event (const SDL_Event &event) {return MF_MLine::deal_w_Event(event);}
    virtual bool deal_w_motion(const SDL_MouseMotionEvent &motionevent);
    virtual bool deal_w_key(const SDL_KeyboardEvent &keyboardevent);
    virtual bool deal_w_click(const SDL_MouseButtonEvent &buttonevent);
    /* pour monter le curseur ou le descendre :) */
    virtual void montecurseur(int crans);
    virtual void descendcurseur(int crans);
    // pour replacer la barre en fonction du curseur
    virtual void rebarre();
    virtual void updatebarre();
    //highlight
    virtual bool clearHighlight();
    virtual void startHighlight();
    virtual void erase(int depart, int nombre=-1);
    //debug
    void printstatut();
    //diff /=> mtext: efface dernieres lignes au lieu des premi�res
    virtual void update_capacity();
    //curseur
    virtual void set_abspos_curseur(int pos);
    virtual int get_offset_curseur();
};

inline MF_Line::MF_Line()
        :updated(false)
{
    setTextColor(0xFF);
}

inline void MF_Line::write(const char* texte, int pos)
{
    std::string tmp = texte;
    write(tmp, pos);
}

inline void MF_Line::write(char texte, int pos)
{
    std::string tmp = "";
    tmp.push_back(texte);
    write(tmp, pos);
}

inline void MF_Line::write(const std::string &texte)
{
    write(texte, this->texte.length());
}

inline void MF_Line::write(const char* texte)
{
    std::string tmp = texte;
    write(tmp);
}

inline void MF_Line::write(char texte)
{
    std::string tmp = "";
    tmp.push_back(texte);
    write(tmp);
}

/* Pour en erase */
/* efface tout � partir de depart */
inline void MF_Line::erase(int depart)
{
    texte.erase(depart);
    set_updated();;
}
/* et nombre de caract�re � partir de d�part */
inline void MF_Line::erase(int depart, int nombre)
{
    texte.erase(depart, nombre);
    set_updated();;
}

/* Pour displayr */
inline void MF_Line::display(Surface &surface)
{
    /* On fait juste un blit */
    if (!updated) refresh();
    this->surface.blitTo(surface, 0, dims);
}

inline MF_WLine::MF_WLine()
        :second_update(1), curseury(0), ecrivable(true), curseurOn(true), clicOn(false),
        surligneOn(false), curseurDisp(true), lastCursor(SDL_GetTicks())
{
}

/* destructeur */
inline MF_WLine::~MF_WLine()
{
    /* rien de sp�cial */
}

inline void MF_WLine::write(const char* texte, int pos)
{
    std::string tmp = texte;
    write(tmp, pos);
}

inline void MF_WLine::write(char texte, int pos)
{
    std::string tmp = "";
    tmp.push_back(texte);
    write(tmp, pos);
}

inline void MF_MLine::write (const char *texte, unsigned int posx, int posy)
{
    std::string tmp(texte);
    write(tmp, posx, posy);
}

inline void MF_MLine::write (char texte, unsigned int posx, int posy)
{
    std::string tmp("");
    tmp.push_back(texte);
    write(tmp, posx, posy);
}

inline void MF_MLine::setcapacity(int capacite)
{
    #ifdef DEBUG_ON
    assert(capacite > 0);
    #endif
    capacity = capacite;
    update_capacity();
}

inline unsigned int MF_MLine::get_nblignes() const
{
    return poslignes.size();
}

inline int MF_MLine::getbarrey(int barreh) const
{
    int maxlines = maxlignes();
    int difference = get_nblignes() - maxlines;

    return 16 + ((dims.h - barreh - 32) * ncurrentligne) / difference;
}

inline int MF_MLine::maxwidth() const
{
    return dims.w-(barreOn==true)*16;
}

/* r�cup�rer la pos pour �crire apr�s le dernier caract�re */
inline xy MF_MLine::get_abspos() const
{
    return xy(get_nblignes()-1, poslignes[get_nblignes()-1].y);
}

/* r�cuperer l'offset d'un caract�re � partir de sa pos */
inline int MF_MLine::get_offset(int x, int y) const //no de ligne + curseury
{
    return poslignes[x].x + y;
}

inline int MF_MLine::get_offset(const xy &abspos, int y) const //ligne + curseury
{
    return abspos.x + y;
}

inline int MF_MLine::get_offset(const xy &abspos) const //fin de ligne
{
    return abspos.x +abspos.y;
}

inline int MF_MLine::get_offset(int x) const //debut de ligne
{
    return poslignes[x].x;
}

/* r�cup�rer l'offset pour �crire apr�s le dernier caract�re */
inline int MF_MLine::get_offset() const
{
    return get_offset(get_abspos());
}

inline xy::xy(int x, int y)
        :x(x), y(y)
{
}

inline bool operator == (xy &a, xy &b)
{
    return (a.x == b.x && a.y == b.y);
}

inline MF_MWLine::MF_MWLine()
        :curseurx(0), curseurxDeb(0)
{
}

//Rien du tout :D
inline MF_MWLine::~MF_MWLine()
{
}

inline void MF_MWLine::write (const char *texte, unsigned int posx, unsigned int posy)
{
    std::string tmp(texte);
    write(tmp, posx, posy);
}

inline void MF_MWLine::write (char texte, unsigned int posx, unsigned int posy)
{
    std::string tmp;
    tmp.push_back(texte);
    write(tmp, posx, posy);
}

inline int MF_MWLine::get_offset_curseur()
{
    return poslignes[curseurx].x + curseury;
}

inline void MF_MWLine::set_abspos_curseur(int pos)
{
    xy a = get_abspos(pos);
    curseurx = a.x;
    curseury = a.y;
}



#endif

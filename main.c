#include "raylib.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define FE 100
#define H (1./FE) /* Pas d'échantillonnage */

#define NBM 20
#define NBL 19

typedef struct _pmat_ 
{
	double m; /* Masse */
	Vector2 pos; /* Position */
	Vector2 vit; /* Vitesse */
	Vector2 frc; /* Accumulateur de force */
	void (*setup)(struct _pmat_ *, double h); /* Intégrateur */
} _pmat_;

typedef struct _link_
{
	double k, l0,z; /* Paramètres k raideur z viscosité l0 longueur à vide*/
	double f; /* force constante */
	struct _pmat_ *M1,*M2; /* Particules */
	void (*setup)(struct _link_ *); /* Calcul des forces */
} _link_ ;

_pmat_ TabM[NBM]; //Particules
_link_ TabL[NBL]; //Ressorts
_link_ L_gravite;

void gravite(struct _link_ * L){
	for (int i=0;i<NBM;i++){
		TabM[i].frc.y += L->f;
	}
}

void setup_frein(struct _link_ *L)
{
	Vector2 f;
	f.x = -(L->z) * (L->M2->vit.x - L->M1->vit.x);
	f.y = -(L->z) * (L->M2->vit.y - L->M1->vit.y);
	
	L->M1->frc.x += f.x;
	L->M1->frc.y += f.y;

	L->M2->frc.x -= f.x;
	L->M2->frc.y += f.y;
}

void setup_ressort(struct _link_ *L){
	double f;
	int x1 = L->M1->pos.x;
	int x2 = L->M2->pos.x;
	int y1 = L->M1->pos.y;
	int y2 = L->M2->pos.y;

	double longueur_courante = sqrt(
				((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1))
	);

	//longueur_courante = y2 - y1;

	f = L->k * ((longueur_courante) - L->l0);

	Vector2 dir;
	dir.x = x2-x1;
	dir.y = y2-y1;

	L->M1->frc.y += f * dir.y;
	L->M2->frc.y -= f * dir.y;
	L->M1->frc.x += f * dir.x;
	L->M2->frc.x -= f * dir.x;

	setup_frein(L);
}

void setup_particule(struct _pmat_ *M,double h)
{
	//M->vit = M->vit+(M->frc/M->m)*h;
	M->vit.x = M->vit.x + (M->frc.x/M->m)*h;
	M->vit.y = M->vit.y + (M->frc.y/M->m)*h;

	M->pos.x += M->vit.x *h;
	M->pos.y += M->vit.y *h;

	M->frc.x = 0;
	M->frc.y = 0;
}

void setup_point_fixe(struct _pmat_ *M,double h)
{
	M->frc.x = 0;
	M->frc.y = 0;
}

void moteur_physique(void){
	for (_link_ *l = TabL;l<TabL+NBL;l++)
	{
		l->setup(l);
	}

	for (_pmat_ *m = TabM;m<TabM+NBM;m++)
	{
		m->setup(m,H);
	}
	
	gravite(&L_gravite);
}

int main(void)
{
	srand(time(NULL));
	
	L_gravite.f = 0;

	for (int i=0;i<NBM;i++){
		TabM[i].m = 50+(rand()%200);
		TabM[i].pos.x = 10+(800/NBM)*i;
		TabM[i].pos.y = 225;
		TabM[i].vit.x = 0;
		TabM[i].vit.y = 0;
		TabM[i].frc.x = 0;
		TabM[i].frc.y = 0;

		if (i ==0 || i == NBM -1)
			TabM[i].setup = setup_point_fixe;
		else
			TabM[i].setup = setup_particule;
	}

	for (int i=0;i<NBL;i++){
		TabL[i].k = 0.866086*sqrt(FE);
		TabL[i].z = (0.0001*FE);
		TabL[i].M1 = &TabM[i];
		TabL[i].M2 = &TabM[i+1];
		TabL[i].l0 = TabL[i].M2->pos.x - TabL[i].M1->pos.x;
		TabL[i].setup = setup_ressort;
	}


    InitWindow(800, 450, "raylib [core] example - basic window");
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
						L_gravite.f = GuiSlider((Rectangle){96,48,216,16},TextFormat("Gravite %0.2f",L_gravite.f),NULL,L_gravite.f,0.0f,10.f);

						for (int i=0;i<NBM;i++){
							DrawCircleLines(TabM[i].pos.x,TabM[i].pos.y,TabM[i].m/10,PINK);
							if (i < NBM-1)
								DrawLine(TabM[i].pos.x,TabM[i].pos.y,TabM[i+1].pos.x,TabM[i+1].pos.y,GREEN);
						}
						moteur_physique();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

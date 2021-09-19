/*
Name:       Cristopher Melendez
Assignment: Assignment 1
Class:      COP 3502C 0V02
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sp21_cop3502_as1.h"
#include "leak_detector_c.h"


/* As given by Professor Gerber*/
void remove_crlf(char *s)
{
    char *t = s + strlen(s);
    t--;
    while((t >= s) && (*t == '\n' || *t == '\r'))
    {
        *t = '\0'; //...clobber it by overwriting it with null, and....
        t--;       //decrement t *itself (i.e., change the pointed location)
    }
}

/* As given by Professor Gerber*/
int get_next_nonblank_line(FILE *ifp, char *buf, int max_length)
{
    buf[0] = '\0';

    while(!feof(ifp) && (buf[0] == '\0'))
    {
        fgets(buf, max_length, ifp);
        remove_crlf(buf);
    }

    if(buf[0] != '\0')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* The following three functions read in the number of monsters
for the index, the number of regions given, and the number
of itineraries that is the same as the number of trainers. */
int get_num_monsters(FILE *ifp)
{
    char buf[256];
    int nummonsters;

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d monsters", &nummonsters);   //Get number of monsters

    return nummonsters; //Return that number
}

int get_num_regions(FILE *ifp)
{
    char buf[256];
    int numregions;

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d regions", &numregions); //Get number of regions

    return numregions;  //Return that number
}

int get_num_its(FILE *ifp)
{
    char buf[256];
    int numits;

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d Trainers", &numits);    //Get number of itineraries/trainers

    return numits;  //Return that number
}

/* This is the creation of the monster index. */
void fill_index(FILE *ifp, monster *m)
{
    char buf[256];
    char namebuf[256];
    char elebuf[256];
    int  population;

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%s %s %d", namebuf, elebuf, &population); //Reading in by given input

    m->name = strdup(namebuf);  //Coping that string to the struct we will be pointing at.
    m->element = strdup(elebuf);
    m->population = population;
}

//Calls the function to fill the index in
void monster_loop(FILE *ifp, monster *m, int n)
{
    for (int i=0; i < n; i++)
    {
        fill_index(ifp, m + i);
    }
}

/* Used to find from the monster index the monsters wanted
for each region. Needed to be able to link the monsters from
the index to the region where they will be.*/
int find_index(char *s, monster *m, int n)
{
    int i;
    for (i=0; i < n; i++)
    {
        if (strcmp(s, m[i].name) == 0)
        {
            return i;
        }
    }
    return 0;
}

/* Needed to get the total population for a region by
adding the individual population of the monsters that have
been pointed to per region.*/
int region_population(monster *m, int n, int *d)
{
    int j=0;
    for (int i=0; i < n; i++)
    {
        int c = d[i];
        j += m[c].population; //Looking at the struct it points to for the population and adding up.
    }
    return j;
}

/* Creates the pointer array from each region to
the monsters that exist in each region.*/
void linking_monsters(region *r, monster *m, int *d, int n)
{
    for (int i=0; i < n; i++)
    {
        int c = d[i];
        r->monsters[i] = &m[c]; //Points to the wanted monster
    }
}

/* Fill the regions struct by reading in each region name and
the number of monsters. Then allocates space to be able to
point to the monsters in the index that will inhabit each region.
In the end the info of the region name, population, and number of
monsters is passed through along with the array of pointers that
point to the monsters in the region.*/
void fill_regions(FILE *ifp, region *r, monster *m, int n)
{
    char buf[256];
    char namebuf[256];
    int nummonsters;
    char monname[256];
    int store[256];
    int i=0, k=0,total=0;

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%s", namebuf);
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d monsters", &nummonsters);
    r->monsters = malloc(nummonsters * sizeof(monster *)); //To point to the monster index for monsters per region

    for(i=0; i < nummonsters; i++)
    {
        get_next_nonblank_line(ifp, buf, 255);
        sscanf(buf, "%s", monname);
        k = find_index(monname,m,n);
        store[i] = k;
    }

    linking_monsters(r,m,store,nummonsters);
    total = region_population(m, nummonsters, store);
    r->total_population = total;
    r->name = strdup(namebuf);
    r->nmonsters = nummonsters;
}

//Loops through to fill region struct
void regions_loop(FILE *ifp, region *r, monster *m, int n, int c)
{
    for (int i=0; i < n; i++)
    {
        fill_regions(ifp, r + i, m, c);
    }
}

//Determines what region the trainer is looking at
int find_region(char *s, region *r, int n)
{
    int i=0;
    for (i=0; i < n; i++)
    {
        if (strcmp(s, r[i].name) == 0)
        {
            return i;
        }
    }
    return 0;
}

//Links to the regions the trainer is going to visit
void linking_regions(itinerary *t, region *r, int *d, int n)
{
    for (int i=0; i < n; i++)
    {
        int c = d[i];
        t->regions[i] = &r[c];
    }
}

/* Fills the itinerary and trainer struct.
Reads in the given data and points per trainer to the
regions they want to visit.*/
void fill_its(FILE *ifp, itinerary *t, trainer *tr, region *r, int n)
{
    char buf[256];
    char namebuf[256];
    char regname[256];
    int store[256];
    int captures;
    int numregions;
    int i,k=0;


    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%s", namebuf);
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d captures", &captures);
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d regions", &numregions);

    t->regions = malloc(numregions * sizeof(region *));
    for (i=0; i < numregions; i++)
    {
        get_next_nonblank_line(ifp, buf, 255);
        sscanf(buf, "%s", regname);
        k = find_region(regname, r, n);
        store[i] = k;
    }

    linking_regions(t,r,store,numregions);

    t->captures = captures;
    t->nregions = numregions;
    tr->name = strdup(namebuf);
    tr->visits = t;
}

//Loops trough to fill itinerary and trainers struct
void its_loop(FILE *ifp, itinerary *t, trainer *tr, region *r, int n, int c)
{
    for (int i=0; i < n; i++)
    {
        fill_its(ifp, t + i, tr + i, r, c);
    }
}

/*The big reveal. Per trainer gives their info,their itinerary,
and to what regions and monsters they were linked.
Some math to determine how many of each monster they will
catch to give them their desired data.*/
void print_trainers(trainer *tr, FILE *ofp)
{
    int i=0,j=0,e=0;
    double k=0;
    double d=0;
    printf("%s\n", tr->name);   //Name of trainer
    fprintf(ofp,"%s\n", tr->name);
    for (i=0; i < tr->visits->nregions; i++)
    {
        printf("%s \n",tr->visits->regions[i]->name);   //Name of region followed by monster data before looping
        fprintf(ofp,"%s\n", tr->visits->regions[i]->name);
        for (j=0; j < tr->visits->regions[i]->nmonsters; j++)
        {
            k = (double)tr->visits->regions[i]->monsters[j]->population / tr->visits->regions[i]->total_population; //To determine commonality
            e = tr->visits->captures;
            d = (double)k*e;    //Commonality multiplied by desired number of catches
            d = round(d);
            if (d >= 1)         //To not see any monsters that will have zero captures
            {
                printf("%.0lf %s\n",d, tr->visits->regions[i]->monsters[j]->name);  //Giving capture name and monster
                fprintf(ofp,"%.0lf %s\n",d, tr->visits->regions[i]->monsters[j]->name);
            }
        }
    }
    printf("\n"); //To have a space between trainers
    fprintf(ofp,"\n");
}

//Loops through the printing of the wanted data
void trainers_loop(trainer *tr,FILE *ofp,int n)
{
    for (int i=0; i < n; i++)
    {
        print_trainers(tr + i,ofp);
    }
}

/*The next four functions go through and free
the arrays that were allocated to point from one
struct to another and the space allocated by strdup*/
void free_monsters(monster *m)
{
    free(m->name);
    free(m->element);
}

void free_regions(region *r)
{
    free(r->name);
    free(r->monsters);
}

void free_its(itinerary *t)
{
    free(t->regions);
}

void free_trainers(trainer *tr)
{
    free(tr->name);
    free(tr->visits);
}

//Loops through the freeing of the structs data
void free_loop(monster *m, region *r, itinerary *t, trainer *tr, int n, int c, int d)
{
    int i;
    for (i=0; i < n; i++)
    {
        free_monsters(m + i);
    }
    for (i=0; i < c; i++)
    {
        free_regions(r + i);
    }
    for (i=0; i < d; i++)
    {
        free_its(t+ i);
    }
}


int main()
{
    atexit(report_mem_leak);
    FILE *ifp;
    FILE *ofp;
    int nummonsters, numregions, numits;
    monster *monsters;
    region *regions;
    itinerary *its;
    trainer *trainers;

    ifp = fopen("cop3502-as1-input.txt", "r");
    ofp = fopen("cop3502-as1-output-melendez-cristopher.txt", "w");

    if (ifp != NULL)
    {}
    else
    {
        printf("Failed to open file\n");
    }

    //Gather the data for the monster index and store it
    nummonsters = get_num_monsters(ifp);
    monsters = malloc(sizeof(monster) * nummonsters);
    monster_loop(ifp,monsters,nummonsters);

    //Gather the data for the region struct and store it
    numregions = get_num_regions(ifp);
    regions = malloc(sizeof(region) * numregions);
    regions_loop(ifp,regions,monsters,numregions,nummonsters);

    /*Gather the data for the itinerary and trainer struct,
    store it, link it together and print it out. */
    numits = get_num_its(ifp);
    its = malloc(sizeof(itinerary) * numits);
    trainers = malloc(sizeof(trainer) *numits);
    its_loop(ifp, its, trainers, regions, numits, numregions);
    trainers_loop(trainers,ofp, numits);

    //Free the allocated memory from within the structs and the structs themselves
    free_loop(monsters, regions, its, trainers, nummonsters, numregions, numits);
    free(monsters);
    free(regions);
    free(its);
    free(trainers);
    fclose(ifp);
    fclose(ofp);

    return 0;
}

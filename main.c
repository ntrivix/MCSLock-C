#include <stdio.h>
#include <stdlib.h>
#include "domaci1.h"

typedef struct node
{
    struct node* next;
    struct node* prev;
    double startTime;
    double timeout;
    int status;
} node;


node* pok = NULL;

int lock_n_threads_with_timeout(int id, int* local, double timeout) {
    double startTime = lrk_get_time();

    node* newNode = malloc(sizeof(node));
    local[0] = newNode;
    newNode->status = 1;
    newNode->startTime = startTime;
    newNode->timeout = timeout;
    node* prevNode = (node*)lrk_get_and_set((int*)&pok, (int)newNode);

    if (prevNode != NULL) {
        prevNode->next = newNode;
    } else {
        //proveriti da li je tajmoutova
        newNode->status = 0;
        if (isNodeTimeout(newNode)) {
            if (lrk_compare_and_set(&(newNode->status), 0, 2))
                    propagateUnlock(newNode);
            if (newNode->status == 2)
                    return 0;
        }
    }
    while ( newNode->status == 1 ){
        if (isNodeTimeout(newNode)) {
            //ako je kroz propagate status promenjen na 0
            if (!lrk_compare_and_set(&(newNode->status), 1, 2)) {
                if (newNode->status == 2)
                    return 0;
                break;
            }
            return 0;
        }
        lrk_sleep(1);
    };
    if (isNodeTimeout(newNode)) {
        //ovooo
        //sta ako je setovano na 0
        if (newNode->status == 0){
            
            // Ovo mozda treba if (newNode->next != NULL) {
            propagateUnlock(newNode);
         
        }
        return 0;
    }
    return 1;
}

void propagateUnlock(node* tNode) {

    node* nextNode;
    do {
        node* nextNode = tNode->next;
        //Ako je next null znaci da smo na kraju liste
        if (nextNode == NULL) {
            //Setuj pok da ukazuje na null jer vise nista ne
            //postoji tred koji ceka.
            //Ovo setovanje mora biti atomicno jer je moguce
            //da se u tom trenutku doda novi tred
            int success = lrk_compare_and_set(&pok, tNode, NULL);

            //ako operacija nije uspesno izvrsena znaci da je neki
            //drugi tred postavio pok na sebe
            if (!success){
                //cekaj da taj tred sebe postavi kao next
                while (nextNode == NULL)
                    lrk_sleep(1);
                //unlokuj ili tajmotuj sledeceg
                unlockOrTimeoutNode(nextNode);
            }

        } else {
            unlockOrTimeoutNode(nextNode);
        }
        tNode = nextNode;
    } while (tNode != NULL && tNode->status == 2);
}

int isNodeTimeout(node* tNode) {
    if (lrk_get_time() - tNode->startTime > tNode->timeout)
        return 1;
    return 0;
}

/*
*
* Ako je node jos uvek na 1, a nije tajmoutovao, tajmoutuj ga.
* Pokušaj da unlokuješ node ako nije tajmoutovao
*
*Vraca 0 ako je node unlokovan, a 2 ako je tajmoutovan 
*/
int unlockOrTimeoutNode(node* tNode) {
    int success;
    if (isNodeTimeout(tNode)) {
        success = lrk_compare_and_set(&(tNode->status), 1, 2);
        if (success)
            return 2;
        return 0;
        // Da li je moguce da ovde fejluje i da je tNode->status == 0 i sta se onda desava
    } else {
        success = lrk_compare_and_set(&(tNode->status), 1, 0);
        if (success)
            return 0;
        return 2;
        // Da li je moguce da ovde fejluje i sta ako je fejlovao, a
        //tNode->status == 2
    }
}

void unlock_n_threads_with_timeout(int id, int* local) {
    node* myNode = (node*)local[0];
    propagateUnlock(myNode);
}

int main()
{

    /*
    node* node1 = malloc(sizeof(node));
    node1->status = 10;
    int* nodeAdr = (int*)&node1;

    node* node3 = malloc(sizeof(node));
    node3->status = 55;

    int node3Adr = (int)node3;
    *nodeAdr = node3Adr;
    node* test = (node*)(*nodeAdr);


    printf("%d\n", test->status);
    */
/*
    node* node1 = NULL;
    //node1->status = 10;

    node* node3 = malloc(sizeof(node));
    node3->status = 55;

    node* prevNode = (node*)lrk_get_and_set((int*)&node1, (int)node3);
    if (prevNode == NULL)
        printf("NULL\n");
    //printf("%d\n", prevNode->status);
    printf("%d\n", node1->status);
    */
    start_timeout_mutex_n_threads_test(0.05f);
    return 0;
}

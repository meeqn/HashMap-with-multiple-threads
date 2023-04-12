#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define OPERATIONS 1000
#define WORKERS 5
#define MAX_SIZE 10

typedef struct Node {
    void* value;
    struct Node* next;
} Node;

typedef struct List {
    Node* subListHeadNode;
    Node* subListTailNode;
    struct List* nextSubList;
    unsigned int index;
} List;

typedef struct HashList {
    pthread_mutex_t p_m;
    List* HashListHead;
    List* HashListTail;
    int size;
    int (*hashingFunc)(void*,int);
    int maxSize;
} HashList;

typedef struct args {
    int id;
    HashList* hashList;
} args;

HashList* makeHashList(int (*hashFunc)(void*, int), int max_size) {
    HashList* newHashList = malloc(sizeof(HashList));
    newHashList->HashListHead = NULL;
    newHashList->HashListTail = NULL;
    newHashList->size = 0;
    newHashList->hashingFunc = hashFunc;
    newHashList->maxSize = max_size;
    pthread_mutex_init(&(newHashList->p_m), NULL);
    return newHashList;
}

void pushBackNode(List* list, Node* newNode) { //push new node to given list
    if (list->subListHeadNode == NULL) {
        list->subListHeadNode = newNode;
        list->subListHeadNode->next = NULL;
        list->subListTailNode = list->subListHeadNode;
    }
    else {
        list->subListTailNode->next = newNode;
        list->subListTailNode = newNode;
        list->subListTailNode->next = NULL;
    }
}

Node* makeNewNode(void* val) {
    Node* freshNode = malloc(sizeof(Node));
    freshNode->value = val;
    freshNode->next = NULL;
    return freshNode;
}
int power(int degree, int base) {
    if (degree == 0)
        return 1;
    else {
        return base*power(degree - 1, base);
    }
}

void freeNode(Node** node) {
    free((*node)->value);
    free(*node);
    *node=NULL;
}

void freeList(List** list) {
    Node* tmpNode = (*list)->subListHeadNode;
    Node* nextNode;
    while (tmpNode != NULL) {
        nextNode = tmpNode->next;
        freeNode(&tmpNode);
        (*list)->subListHeadNode = tmpNode;
        tmpNode = nextNode;
    }
    free(*list);
    *list=NULL;
}

void freeHashList(HashList** hashList) {
    List* tmpSublist = (*hashList)->HashListHead;
    List* nextSublist;
    while (tmpSublist != NULL) {
        nextSublist = tmpSublist->nextSubList;
        freeList(&tmpSublist);
        (*hashList)->HashListHead = tmpSublist;
        tmpSublist = nextSublist;
    }
    (*hashList)->HashListTail = NULL;
    (*hashList)->HashListHead = NULL;
    (*hashList)->size=0;
}

void* encryptValueToDesiredType(int value) {
     int* saveVal = malloc(sizeof(saveVal));
    *saveVal = value;
    return (void*)saveVal;}

int  decryptFromDesiredType(void* value) {
    return (*(int*)value);
}

List* newIndex(HashList* hashList, unsigned int index) { //push list with new index to hashlist
    List* newList = malloc(sizeof(List));
    newList->subListHeadNode= NULL;
    newList->subListTailNode = NULL;
    newList->nextSubList = NULL;

    List* currList = hashList->HashListHead;
    if (hashList->HashListHead == NULL) {
        hashList->HashListHead = newList;
        hashList->HashListHead->nextSubList = NULL;
        hashList->HashListTail = hashList->HashListHead;
    }
    else if (index > hashList->HashListTail->index) {
        hashList->HashListTail->nextSubList = newList;
        hashList->HashListTail = newList;
        newList->nextSubList = NULL;
    }
    else if (index < hashList->HashListHead->index) {
        newList->nextSubList = hashList->HashListHead;
        hashList->HashListHead = newList;
    }
    else {
        do {
            if (currList->index < index && currList->nextSubList->index>index) {
                newList->nextSubList = currList->nextSubList;
                currList->nextSubList = newList;
                break;
            }
            currList = currList->nextSubList;
        } while (currList->nextSubList != NULL);
    }
    newList->index = index;
    hashList->size += 1;
    return newList;
}

void drawListNodes(List* list) {
    if (list->subListHeadNode != NULL) {
        Node* currNode = list->subListHeadNode;
        while (1) {
            if (currNode == list->subListTailNode) {
                printf("->|%d|\n", decryptFromDesiredType(currNode->value));
                return;
            }
            printf("->|%d|", decryptFromDesiredType(currNode->value));
            currNode = currNode->next;
        }
    }
    else {
        printf("\n");
    }
}

void drawHashList(HashList* hashList) {
    if (hashList->HashListHead!=NULL) {
        List* currIndexSublist = hashList->HashListHead;
        while (1) {
            if (currIndexSublist == hashList->HashListTail) {
                printf("[%d]", currIndexSublist->index);
                drawListNodes(currIndexSublist);
                return;
            }
            printf("[%d]", currIndexSublist->index);
            drawListNodes(currIndexSublist);
            currIndexSublist = currIndexSublist->nextSubList;
        }
    }
    else{
        printf("NULL hashList \n");
    }
}

void deleteFromHashList(void* key, HashList** hashMap){
    HashList* hashList = *hashMap;
    int index = hashList->hashingFunc(key, hashList->maxSize);
    List* tmpList = hashList->HashListHead;
    List* prevList = NULL;
    while(tmpList!=NULL){
        if(tmpList->index==index){

            Node* tmpNode = tmpList->subListHeadNode;
            Node* prevNode = NULL;
            while(tmpNode!=NULL){
                if(decryptFromDesiredType(tmpNode->value) == decryptFromDesiredType(key)){
                    if(tmpNode==tmpList->subListHeadNode && tmpNode==tmpList->subListTailNode){
                        if(tmpList==hashList->HashListTail && tmpList==hashList->HashListHead){
                            freeHashList(hashMap);
                            break;
                        }
                        else if(tmpList==hashList->HashListHead){
                            hashList->HashListHead = tmpList->nextSubList;
                            freeList(&tmpList);
                            hashList->size=hashList->size-1;
                            break;
                        }
                        else if(tmpList==hashList->HashListTail){
                            hashList->HashListTail = prevList;
                            prevList->nextSubList=NULL;
                            freeList(&tmpList);
                            hashList->size=hashList->size-1;
                            break;
                        }
                        else{
                            prevList->nextSubList = tmpList->nextSubList;
                            freeList(&tmpList);
                            hashList->size=hashList->size-1;
                            break;
                        }
                    }
                    else if(tmpNode==tmpList->subListHeadNode){
                        tmpList->subListHeadNode = tmpNode->next;
                    }
                    else if(tmpNode == tmpList->subListTailNode){
                        tmpList->subListTailNode = prevNode;
                        prevNode->next = NULL;
                    }
                    else{
                        prevNode->next = tmpNode->next;
                    }
                    freeNode(&tmpNode);
                    break;
                }
                prevNode = tmpNode;
                tmpNode=tmpNode->next;
            }
            break;
        }
        prevList = tmpList;
        tmpList=tmpList->nextSubList;
    };
}


int hashFunction(void * val, int max_size){
    int sum = 0;
    int j = 0;
    for (int i = 0; i < 8*sizeof(decryptFromDesiredType(val)); i++) {
        int mask = 1 << i;
        int maskedVal = decryptFromDesiredType(val) & mask;
        int theBit = maskedVal >> i;
        if (theBit == 1) {
            sum += 1*power(i, 2);
        }

    }
    return sum % max_size;
}

void addToHashList(HashList*hashList, void* val) { 
    List* tmpList;
    Node * newNode = makeNewNode(val);
    int index = hashList->hashingFunc(newNode->value, hashList->maxSize);

    if (hashList->HashListHead==NULL || index<hashList->HashListHead->index || index>hashList->HashListTail->index) {
        tmpList=newIndex(hashList, index);
        pushBackNode(tmpList, newNode);
    }
    else {
        tmpList = hashList->HashListHead;
        while (1) {
            if (tmpList->index == index) {
                pushBackNode(tmpList, newNode);
                return;
            }
            else if (tmpList->index<index && tmpList->nextSubList->index>index) {
                tmpList = newIndex(hashList, index);
                pushBackNode(tmpList, newNode);
                return;
            }
            tmpList = tmpList->nextSubList;
        }
    }
}

void* childGoesToWork(void*arg){
    args* arguments = (args*)arg;
    const int id = arguments->id;
    HashList* hashList = arguments->hashList;
    for(int i=0; i<OPERATIONS; i++){
        int val = 1000*id+ i;
        pthread_mutex_lock(&(hashList->p_m));
        addToHashList(hashList, encryptValueToDesiredType(val));
        pthread_mutex_unlock(&(hashList->p_m));

        pthread_mutex_lock(&(hashList->p_m));
        deleteFromHashList(encryptValueToDesiredType(val), &hashList);
        pthread_mutex_unlock(&(hashList->p_m));
    }
}
void destroyHashList(HashList**list){
    if(*list!=NULL){
        pthread_mutex_destroy(&((*list)->p_m));
        freeHashList(list);
        free(*list);
        *list = NULL;
    }
}

int main() {
    HashList* hashMap = makeHashList(hashFunction, (int)MAX_SIZE);
    pthread_t threads[WORKERS];
    args arg[WORKERS];
    addToHashList(hashMap, encryptValueToDesiredType(0));
    for(int i=0; i<WORKERS; i++){
        arg[i].hashList = hashMap;
        arg[i].id = i+1;
        pthread_create(&threads[i], NULL, childGoesToWork, (void*)&arg[i]);
    }
    for(int i=0; i<WORKERS; i++){
        pthread_join (threads[i], NULL);
    }
    drawHashList(hashMap);
    deleteFromHashList(encryptValueToDesiredType(0), &hashMap);
    drawHashList(hashMap);
    destroyHashList(&hashMap);
    printf("finished \n");
    return 0;
}

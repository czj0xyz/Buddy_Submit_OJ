#include "buddy.h"
#include <stdlib.h>
#include <stdio.h>
#define NULL ((void *)0)

#define MAXRANK 16

struct list_Node{
    void* ptr;
    struct list_Node* nex;
    // struct list_Node((void*) ptr_) : ptr(ptr_),nex(NULL){}
    // struct list_Node((void*) ptr_,struct list_Node* nex_) : ptr(ptr_),nex(nex_){}
};
typedef struct list_Node listNode;
listNode* Used[MAXRANK];
listNode* Val[MAXRANK];

void Insert(listNode* head,listNode* v){
    listNode* u = head;
    while(u->nex != NULL && u->nex->ptr < v->ptr) u = u->nex;
    v->nex = u->nex;
    u->nex = v;
}

int init_page(void *p, int pgcount){
    for(int i=0;i<MAXRANK;i++){
        Used[i] = (listNode* )malloc(sizeof(listNode));
        Used[i]->nex = NULL;
        Val[i] = (listNode* )malloc(sizeof(listNode));
        Val[i]->nex = NULL;
    }
    // printf("%d %d\n", pgcount,1<<MAXRANK);
    for(int i=MAXRANK-1,now=pgcount;i>=0;i--){
        listNode* node = Val[i];
        while(now>=(1<<i)){
            node->nex = (listNode* )malloc(sizeof(listNode));
            node->nex->ptr = p;
            node->nex->nex = NULL;
            node = node->nex;

            p += (1<<i)*(1<<12);
            now -= 1<<i;
        }
    }
    return OK;
}

void *alloc_pages(int rank){
    if(rank<1||rank>MAXRANK)return (void*)(-EINVAL);
    rank--;
    for(int i=rank;i<MAXRANK;i++)if(Val[i]->nex!=NULL){
        listNode* node = Val[i]->nex;
        Val[i]->nex = Val[i]->nex->nex;
        for(int j=i-1;j>=rank;j--){
            Val[j]->nex = (listNode* )malloc(sizeof(listNode));
            Val[j]->nex->ptr = node->ptr+(1<<j)*(1<<12);
        }
        void* ret = node->ptr;

        node->nex = Used[rank]->nex;
        Used[rank]->nex = node;
        return ret;
    }
    return (void*)(-ENOSPC);
}

int return_pages(void *p){
    listNode* node = NULL;
    int length = 0;
    for(int i=0,brk=0;!brk&&i<MAXRANK;i++){
        for(listNode* u = Used[i];!brk&&u->nex;u=u->nex)if(u->nex->ptr==p){
            node = u->nex;
            u->nex = u->nex->nex;
            length = i;
            brk = 1;
        }
    }
    if(node == NULL) return -EINVAL;
    Insert(Val[length],node);
    for(int i=length;i<MAXRANK;i++){
        int flg = 0;
        for(listNode* u = Val[i];u->nex&&u->nex->nex;u=u->nex)if(u->nex->ptr+(1<<i)*(1<<12)==u->nex->nex->ptr){
            flg = 1;
            listNode* v = u->nex;
            u->nex = u->nex->nex->nex;
            free(v->nex);
            Insert(Val[i+1],v);
            break;
        }
        
        if(!flg)break;
    }
    return OK;
}

int query_ranks(void *p){
    for(int i=0;i<MAXRANK;i++)
        for(listNode* u = Used[i];u->nex;u=u->nex)
            if(u->nex->ptr<=p&&p<u->nex->ptr+(1<<i)*(1<<12)) return i+1;

    for(int i=MAXRANK-1;i>=0;i--)
        for(listNode* u = Val[i];u->nex;u=u->nex)
            if(u->nex->ptr<=p&&p<u->nex->ptr+(1<<i)*(1<<12)) return i+1;
    
    return -EINVAL;
}

int query_page_counts(int rank){
    if(rank<1||rank>MAXRANK)return -EINVAL;
    int ret = 0;
    for(listNode* u = Val[rank-1];u->nex;u=u->nex)ret++;
    return ret;
}

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#define DB_MAX 10
#define THREAD_SIZE 5
#define DATA_MAX 100

sem_t W_mutex;           //writer의 mutex --> reader와 writer 모두 접근 x
sem_t R_mutex;           //Reader의 mutex --> writer 접근 x

int readernum = 0;        //reader의 수
int DB[DB_MAX] = {0,};         //DB 공간

void * Writer(void * i)                  
{ 
    srand((unsigned int)time(NULL));  
    int Writer = *(int*)i + 1;
    while(1)
    {
        sem_wait(&W_mutex);         
        int DBwrite = rand() % DB_MAX;      //write하고자하는 data영역 랜덤 선택
        int Data = rand() % DATA_MAX + 1;   //data 랜덤 생성

        DB[DBwrite] = Data;            //Write
        printf("[%d]Writer write data %d in database %d area \n",Writer,Data,DBwrite);
  
       
       sem_post(&W_mutex);
       sleep(1);                       //context switching time 확보
    }
}

void * Reader(void * i)                  
{   
    srand((unsigned int)time(NULL));
    int Reader = *(int*)i + 1;
    while(1)
    {
        sem_wait(&R_mutex);             //readnum증가시 혹시 모를 race condition 방지
        readernum++;
        if(readernum==1) sem_wait(&W_mutex);
        sem_post(&R_mutex); 

        int DBread = rand() % DB_MAX;   //read하고자하는 data영역 랜덤 선택
        int data;                       //읽어드릴 데이터

        data = DB[DBread];
         
        printf("[%d]Reader read data %d of database %d area \n",Reader,data,DBread);
        sem_wait(&R_mutex);
        readernum--;
        if(readernum==0)    sem_post(&W_mutex);
        sem_post(&R_mutex);
        sleep(1);                   //context switching time 확보

    }

}


int main()
{
    pthread_t wthread[THREAD_SIZE];         //writer thread
    pthread_t rthread[THREAD_SIZE];         //reader thread
    pthread_attr_t attr;
    int wi;
    int ri;
    
    if(sem_init(&W_mutex,0,1)==-1)          //writer의 mutex semaphore, 접근 위해 1
    {
        printf("sem_init error!\n");
        exit(-1);
    }

    if(sem_init(&R_mutex,0,1)==-1)          //reader의 mutex semaphore, 접근 위해 1 
    {
        printf("sem_init error!\n");
        exit(-1);
    }

     if(pthread_attr_init(&attr)==-1)
    {
        printf("attr_init error!!\n");
        exit(-1);
    } 

    for(wi = 0; wi < 1 ; wi++)
    {
    if(pthread_create(&wthread[wi],&attr,Writer,(void*)&wi)!=0)  //생산자 생성
    {
        printf("pthread_create error!!\n");
        exit(-1);
    }
    sleep(1);                                                   //인자 전달 시간 확보
    }
	
	for(ri = 0 ; ri < 10 ; ri++){
	if(pthread_create(&rthread[ri],&attr,Reader,(void*)&ri)!=0)  //소비자 생성
    {
        printf("pthread_create error!!\n");
        exit(-1);
    }
    sleep(1);                                                   //인자 전달 시간 확보
    }
	for (wi = 0; wi < 4 ; wi++) 
    {
	  if(pthread_join(wthread[wi],NULL)!=0) 
        {
          printf("pthread_join error!!\n");
          exit(-1);
        }
    }
    for(ri = 0;ri < 10; ri ++)
    {
        if(pthread_join(rthread[ri],NULL)!=0)
        {
            printf("pthread_join error!!\n");
            exit(-1);
        }
    }
    

    pthread_exit(NULL);    
    


    return 0;

}
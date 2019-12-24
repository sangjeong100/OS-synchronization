#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define ITEM_MAX 10
#define THREAD_SIZE 5

sem_t Empty_check;          //생산자의 세마포어
sem_t Item_exist;           //소비자의 세마포어
sem_t mutex;                //상호 배재를 위한 선언

int count = 0;          //공유 변수
int ItemList[ITEM_MAX];     //Item 목록
int in = 0;             //생산
int out = 0;            //소비


void *Producer(void * i)
{
    int producer = *(int *)i + 1;

    while(1)
    {  
        sem_wait(&Empty_check);               //List 채우기 시작 , 다른 프로세스 접근 차단
        sem_wait(&mutex);                      //binary semaphore      
      
        ItemList[in] = in+1;                  //1~10 으로 표현하기 위해서
        printf("[%d]producer produce %d \n",producer,ItemList[in]);
        in = (in+1) % ITEM_MAX;               //in 증가
        
        sem_post(&mutex);                   //다른 생산자가 임계영역 사용 가능
        sem_post(&Item_exist);              //생산을 끝냈으므로 item 공간 증가
        
        sleep(1);                       //context switching time 확보

	}
}

void *Consumer(void * i)
{   
    int consumer = *(int *)i + 1; 
    while(1)
    { 
        sem_wait(&Item_exist);              //Item 소비
        sem_wait(&mutex);

        int item = ItemList[out];   
        printf("[%d]consumer consume : %d\n",consumer,item );
        out =(out + 1) % ITEM_MAX;          //out 증가

        sem_post(&mutex);        
        sem_post(&Empty_check);             //소비를 끝냈으니 빈공간 증가
        sleep(1);                           //context switching time 확보
    }
}

int main()
{
    pthread_t pthread[THREAD_SIZE];
    pthread_t cthread[THREAD_SIZE];
    pthread_attr_t attr;
	int i;
    int pi;

    if(sem_init(&Empty_check,0,ITEM_MAX)==-1) {     //생산자 세마포어, 생성해야하므로 ITEM_MAX로 초기화
        printf("semaphore init error!\n");
        exit(-1);
    }
    if(sem_init(&Item_exist,0,0)==-1)        //소비자 세마포어,처음부터 소비가능하지 않음으로 0으로 초기화
    {
        printf("semaphore init error!\n");
        exit(-1);
    }
    if(sem_init(&mutex,0,1)==-1)            //상호배재를 위한 binary semaphore
    {
        printf("semaphore init error!\n");
        exit(-1);
    }
    
    if(pthread_attr_init(&attr)==-1)
    {
        printf("attr_init error!!\n");
        exit(-1);
    } 

    for(pi = 0; pi<1;pi++)
    {
    if(pthread_create(&pthread[pi],&attr,Producer,(void*)&i)!=0)  //생산자 생성
    {
        printf("pthread_create error!!\n");
        exit(-1);
    }
    sleep(1);               //인자 전달 시간 확보
    }
	
	for(i =0;i<3;i++)
    {
	if(pthread_create(&cthread[i],&attr,Consumer,(void*)&i)!=0)  //소비자 생성
    {
         printf("pthread_create error!!\n");
         exit(-1);
    }
    sleep(1);               //인자 전달 시간 확보
    }
	
    for(pi=0;pi<1;pi++)
    {
        if(pthread_join(pthread[i],NULL)!=0)
        {
            printf("pthread_join error!!\n");
            exit(-1);
        }        
    }

	for (i = 0; i < 3; i++) 
    {
	  if(pthread_join(cthread[i],NULL)!=0) 
      {
          printf("pthread_join error!!\n");
          exit(-1);
      }
    }

    sem_destroy(&Empty_check);
    sem_destroy(&Item_exist);
    sem_destroy(&mutex);

    pthread_exit(NULL);

    return 0;
}
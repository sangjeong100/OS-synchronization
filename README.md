# 운영체제 - 동기화

============================================================================

이 프로젝트는 동기화의 전통적인 문제인 Producer/Consumer 문제와 Writer/Reader 문제를 동기화 기능을 추가하여서 구현 실행하는 프로젝트이다. 

## Producer/Consumer 문제

-------------------------------------

> Producer/Consumer 문제는 유한한 개수의 데이터을 임시로 보관하는 버퍼에 생산자와 소비자가 접근한다. 생산자는 물건이 하나 만들어지면 그 공간에 저장한다. 이때 저장할 공간이 없는 문제가 발생할 수 있다. 소비자는 물건이 필요할 때 보관함에서 물건을 하나 가져온다. 이 때는 소비할 물건이 없는 문제가 발생할 수 있다. 즉, Race condition의 발생을 막도록 동기화를 해주어야 한다. 필자는 동기화를 위하여 세마포어를 사용하였다.



### 동기화 전 코드

``` c
  void *Producer()
{	
	while(1)
	{	while(count > ITEM_MAX);				//생산공간 없으면 stop
		  printf("produce %d \n",count);
		  count++;								//race condition 발생 가능 부분
	}
}  //동기화 전 producer 코드 부분
```

```c
void *Consumer()
{
    while(1)
    {
		while(count <= 0);					//소비할것 없으면 stop
        printf("consume %d\n",count);
        count--;							//race condition 발생가능 부분
    }
}	//동기화 전 consumer 코드 부분
```

위는 동기화 전의 producer와 consumer 부분으로서, 공유변수 count를 이용하여 생산 및 초기화를 표현하고 있다. 그래서 공유변수 **count**를 통해 **race condition**이 발생 할 수 있다.

  ![동기화전 실행코드](D:\study\2학년 2학기\운영체제\homework2\동기화전 실행코드.png)

(동기화전 Race condition 발생 부분)

위에서 보이듯이, consume 10이 두번 출력됨으로서, race condition이 발생했음을 확인 할 수 있다. 

그렇다면, 위 문제를 해결하기 위해서는 동기화 기능을 추가하여야 함을 느낄 수 있다. 그래서 필자는 semaphore 를 사용하여 동기화 기능을 추가하였다.

### 동기화 후 코드

```c
#include<semaphore.h>

sem_t Empty_check;				//생산자의 세마포어
sem_t Item_exist;				//소비자의 세마포어
sem_t mutex;					//상호 배제를 위한 선언
```

```c
void *Producer()			//생산자 코드
{
    while(1)
    {
        sem_wait(&Empty_check);					//빈공간있으면 생산 시작
        sem_wait(&mutex);						//다른 프로세스의 접근 차단
        
        ItemList[in] = in + 1;					
        printf("producer produce %d \n",ItemList[in]);
        in = (in + 1) % ITEM_MAX;				//in 증가
       	
        sem_post(&mutex);						//다른 쓰레드가 임계구역접근가능
        sem_post(&Item_exist);					//생산을 끝냈으므로 item공간증	
        slee(1);						//context switching time 확보
	}
}
```

코드를 보면 semaphore 변수가 세가지가 선언 되었다. 이유는 counting semaphore 2개와 binary semaphore 1개를 나타 내기 위함인데, counting semaphore는 각각 아이템 수와 빈공간 수를 나타내며, binary semaphore는 임계구역 동시접근 차단기능을 행한다. 

sem_wait(&변수) -> 변수 값이 0이아니면 1낮추고 0이라면 대기

sem_post(&변수) -> 변수 값을 1증가 시킴. 

```c
void *Consumer()		//소비자 코드
{
    while(1)
    {
        sem_wait(&Item_exist);					//Item있으면 사용 시작
        sem_wait(&mutex);						//다른 프로세스 접근 차단
        
        int item = ItemList[out];				//item 소비
        printf("Consumer consume %d \n",item);
        out = (out + 1) % ITEM_MAX;				//out 증가
        
        sem_post(&mutex);						//다른 프로세스 접근 허용
        sem_post(&Empty_check);					//소비 후 소비 공간 증가
        sleep(1);					//context switching time 확보
    }
}
```

위의 생산자와 소비자의 코드를 보면 counting semaphore를 사용하여, 아이템 수와 빈 공간의 수가 역관계를 이루고 있다. 그래서 각각이 0이 되면 소비자 차단, 생산자 차단을 하게 된다.  

![P,C동기화후 실행코드](D:\study\2학년 2학기\운영체제\homework2\P,C동기화후 실행코드.png)

(실행의 예) -> 생산자 1, 소비자 2



요약

위 프로그램은 임계구간 해결법 3가지(Mutual Exclusion , Progress , Bounded Waiting)를 만족한다고 할 수 있다.

 첫째, Mutual Exclusion 조건은 Binary Semaphore mutex 변수를 활용하여서 완성하였다.

 둘째, progress 조건은 counting Semaphore 변수인 Empty_check와 Item_exist를 활용하여 생산자 소비자가 각각 역할이 끝나면 임계 영역을 다른 쓰레드가 접근할 수 있도록 하였다.

 셋째, mutex 변수를 활용하여, 생성이나, 소비를 마치면 임계영역의 접근을 허용한다.  그리고 실행 에서 볼 수 있듯이, 생산자 - 소비자순으로 진행되며, 소비자나 생산자가 여럿이면, 생산자 소비자가 각각 순차적으로 실행된다.

이것으로 Producer/Consumer 문제의 Readme를 마치겠다.



## Writer/Reader 문제



----------------------------------------------------------------------------------------------------

>  Reader는 동시 수행 가능하므로, Writer의 기아 현상이 발생 할 수 있다. 그러므로 Reader 수행 중 Writer가 들어오면 Writer를 우선적으로 수행한다. 이때, Reader의 기아 현상(starvation)이 발생 할 수 있다.  또한 Race condition이 발생할 수 있으므로 문제를 해결하기위해 동기화기능을 추가해주어야 한다. 

  #### 참고

> writer가 data write시 writer와 reader모두 접근할 수 없다.
>
> Reader가 data read시 수정이 안되도록 writer는 접근할 수 없고, 다른 reader는 동시 접근이 가능하다. 하지만 동기화 전에는 Writer의 기아현상을 막기위하여, writer가 우선선점하도록 설정하였다.

### 동기화전 코드

```c
void * Writer(void * i)         //                   -->동기화 전
{ 
    srand((unsigned int)time(NULL));  			//랜덤함수
    int Writer = *(int*)i + 1;
    while(1)
    {
        while(writernum>0);         //다른 writer가 점유시 대기 
        int DBwrite = rand() % DB_MAX;      //write하고자하는 data영역 랜덤 선택
        int Data = rand() % DATA_MAX + 1;   //data 랜덤 생성

        writernum++;
        DB[DBwrite] = Data;            //Write
        printf("[%d]Writer write Database %d area \n",Writer,DBwrite);
        writernum--;
        printf("race condition을 확인하기 위한 writernum 출력 : %d\n",writernum);

        sleep(1);                       //context switching time 확보
    }
}
```

writernum을 사용하여서 writer끼리의 동시 접근을 차단한다. 그러나 동기화가 되어있지않아서, 여러 writer가 CPU를 점유하기위해 race condition상황이 발생한다면, Writernum이 잘못된 값으로 나올 수 있다.  올바른 값이라면, printf("race condition을 확인하기 위한 writernum 출력 : %d\n",writernum); 이 부분에서 writernum은 항상 0이 출력되어야 한다.

랜덤함수 사용은 data 랜덤 생성을 위하여 사용하였다.

```c
void * Reader(void * i)              //               --> 동기화 전
{   
    srand((unsigned int)time(NULL));		
    int Reader = *(int*)i + 1;
    while(1)
    {
        while(writernum>0)sleep(3);         //writer가 점유시 대기

    int DBread = rand() % DB_MAX;   //read하고자하는 data영역 랜덤 선택
    int data;                       //읽어드릴 데이터
        readernum++;

        if(writernum>0){            //writer의 점유
            readernum--;
            sleep(1);   
            continue;
        }

        data = DB[DBread];
         if(writernum>0){            //writer의 점유
            readernum--;
            sleep(1);
            continue;
        }
        printf("[%d]Reader read data %d of database %d area\n",Reader,data,DBread);
        readernum--;

        printf("race condition을 확인하기 위한 readernum 출력 : %d\n",readernum);

        sleep(1);                   //context switching time 확보

    }

}
```

동기화 전 reader코드는 writer의 기아현상을 막기위하여, writer우선선점 방식으로 코딩하였다. 그리고 readernum을 사용하여, reader의 수를 사용하였는데, 이 부분에서 여러 Reader가 CPU를 점유하기위해, Race Condition상황이 발생하면, readernum이 잘못된 값이 출력 될 수 있다.



![R,w동기화전](D:\study\2학년 2학기\운영체제\homework2\R,w동기화전.png)

(동기화 전 실행 코드)

위 사진에서 보이듯이,  writernum이 1이 출력된 구간이 발생 함으로서, race condition이 발생했음을 확인 할 수 있다. 또한 writer가 먼저 점유함으로서, reader의 기아현상을 기대하였지만, reader의 빈도수가 적긴하지만, 기아현상까지는 발생하지 않았다. 

그럼 위 Race condition을 해결하기위해서 동기화 기능을 추가하겠다.



### 동기화 후 코드

```c
sem_t W_mutex;				//writer의 mutex --> reader와 writer 모두 접근 x
sem_t R_mutex;				//Reader의 mutex --> writer 접근 x 
```

Writer일떄와 Reader일떄의 경우의 세마포어 변수 두개 선언하였다.

```c
void * Writer(void * i)       			//동기화 후 writer 코드           
{ 
    srand((unsigned int)time(NULL));  
    int Writer = *(int*)i + 1;
    while(1)
    {
        sem_wait(&W_mutex);         		//모든 쓰레드 접근 차단
        int DBwrite = rand() % DB_MAX;      //write하고자하는 data영역 랜덤 선택
        int Data = rand() % DATA_MAX + 1;   //data 랜덤 생성

        DB[DBwrite] = Data;            //Write
        printf("[%d]Writer write data %d in database %d area \n",Writer,Data,DBwrite);
  
       
       sem_post(&W_mutex);
       sleep(1);                       //context switching time 확보
    }
}

```

writer는 sem_wait(&w_mutex); , sem_post(&W_mutex); 를 사용하여 임계구역의 Reader와 writer의 접근을 모두 차단한다.

```c
void * Reader(void * i)        			//동기화 후 reader의 코드          
{   
    srand((unsigned int)time(NULL));
    int Reader = *(int*)i + 1;
    while(1)
    {
        sem_wait(&R_mutex);             //readnum증가시 혹시 모를 race condition 방지
        readernum++;
        if(readernum==1) sem_wait(&W_mutex);	//writer의 접근 차단
        sem_post(&R_mutex); 

        int DBread = rand() % DB_MAX;   //read하고자하는 data영역 랜덤 선택
        int data;                       //읽어드릴 데이터

        data = DB[DBread];
         
        printf("[%d]Reader read data %d of database %d area \n",Reader,data,DBread);
        sem_wait(&R_mutex);
        readernum--;
        if(readernum==0)    sem_post(&W_mutex);	//writer의 접근 허용
        sem_post(&R_mutex);
        sleep(1);                   //context switching time 확보

    }

}

```

reader 코드에서 readernum으로 reader의 수를 증감하는데, readernum의 증감부분에, reader끼리의 race condition을 방지하기위해서, 증감시 wait - post를 해주었다. 그리고 reader는 여러명이 동시에 접근 가능하면서, reader가 점유할때, writer의 접근을 막기위해서 

> ```c
> if(readernum==1) set_wait(&W_mutex);
> .
> .
> .
> if(reaernum == 0) sem_post(&W_mutex);
> 
> ```



위와 같은 코드를 사용 하였다.  이 코드를 통해서 writer는 reader가 점유 할 시, 접근하지 못하고, wait하게 되며, reader가 0이되는 순간, writer가 임계구간에 접근 할 수 있도록 깨워주므로서, writer가 점유하게 된다. 

![r,w 동기화후](D:\study\2학년 2학기\운영체제\homework2\r,w 동기화후.png)

 																(동기화 후 실행 화면)

위 실행 화면을 보면 writer 수행할땐 reader는 접근할 수 없으며, reader 수행할땐, 여러 reader가 동시에 접근하면서 writer는 접근하지 않음을 확인 할 수 있다. 그리고 reader들의 수행을 끝맞치면 writer가 순차적으로 실행되기도 하고, writer사이에 끼어 들기도 한다. 그래서 reader들의 수행이 끝나면 writer가 오는 것은 확실함을 알 수 있다. 다만 writer의 waiting time이 너무 길어지는 것을 방지하기 위해서 reader의 수를 제한해 준다.

  

```c
#define THREAD_SIZE 5 ----> size는 변경 가능

```

	##### 요약
	

> 위 프로그램은 임계구간 해결법 3가지(Mutual Exclusion , Progress , Bounded Waiting)를 만족한다고 할 수 있다.
>
> 첫째, Mutual Exclusion 조건은 Writer일 때, 모든 writer와 reader의 접근을 막는 Binary Semaphore W_mutex 변수를 활용하여서 완성하였다. 또한 reader가 readernum 변수의 접근을 할때, 세마포어 R_mutex변수를 사용하여 각 reader의 race condition을 차단하였다. 그 것을 통해서 상호 배제 조건이 완성됨을 알 수 있다.  
>
> 둘째, progress 조건은 세마포어 변수 R_mutex와 W_mutex를 사용하여 reader는 reader끼리 동시 임계 구간 접근 허용으로 잘 수행되고, writer도 W_mutex를 활용하여 자신의 역할이 끝나면 권한을 열어줌 으로서 기다리고 있는 writer들의 수행이 잘 되도록 한다. 이를 통해 모든 writer와 reader 모두 잘 실행이 된다. 
>
> 셋째, 어느 thread도 무한하게 기다리는 thread는 없다. 물론 reader의 수가 이 가 writer가 CPU를 대기 시간이 수는 있다. 하지만, 시간의 문제일뿐,  일어나지 않는다.					



이를 통해 readme를 마치도록 하겠다. 






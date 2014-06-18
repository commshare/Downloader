#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

/*
*�̳߳����������к͵ȴ���������һ��CThread_worker
*�������������������������һ������ṹ
*/
typedef struct worker
{
    /*�ص���������������ʱ����ô˺�����ע��Ҳ��������������ʽ*/
    void *(*process) (void *arg);
    void *arg;/*�ص������Ĳ���*/
    struct worker *next;

} CThread_worker;


/*�̳߳ؽṹ*/
typedef struct
{
     pthread_mutex_t queue_lock;
     pthread_cond_t queue_ready;

    /*����ṹ���̳߳������еȴ�����*/
     CThread_worker *queue_head;

    /*�Ƿ������̳߳�*/
    int shutdown;
     pthread_t *threadid;
    /*�̳߳�������Ļ�߳���Ŀ*/
    int max_thread_num;
    /*��ǰ�ȴ����е�������Ŀ*/
    int cur_queue_size;

} CThread_pool;


int pool_add_worker (void *(*process) (void *arg), void *arg);
void *thread_routine (void *arg);


static CThread_pool *pool = NULL;
void
pool_init (int max_thread_num)
{
     pool = (CThread_pool *) malloc (sizeof (CThread_pool));

     pthread_mutex_init (&(pool->queue_lock), NULL);
     pthread_cond_init (&(pool->queue_ready), NULL);

     pool->queue_head = NULL;

     pool->max_thread_num = max_thread_num;
     pool->cur_queue_size = 0;

     pool->shutdown = 0;

     pool->threadid =
         (pthread_t *) malloc (max_thread_num * sizeof (pthread_t));
    int i = 0;
    for (i = 0; i < max_thread_num; i++)
     {
         pthread_create (&(pool->threadid[i]), NULL, thread_routine,
                 NULL);
     }
}


/*���̳߳��м�������*/
int
pool_add_worker (void *(*process) (void *arg), void *arg)
{
    /*����һ��������*/
     CThread_worker *newworker =
         (CThread_worker *) malloc (sizeof (CThread_worker));
     newworker->process = process;
     newworker->arg = arg;
     newworker->next = NULL;/*�����ÿ�*/

     pthread_mutex_lock (&(pool->queue_lock));
    /*��������뵽�ȴ�������*/
     CThread_worker *member = pool->queue_head;
    if (member != NULL)
     {
        while (member->next != NULL)
             member = member->next;
         member->next = newworker;
     }
    else
     {
         pool->queue_head = newworker;
     }

     assert (pool->queue_head != NULL);

     pool->cur_queue_size++;
     pthread_mutex_unlock (&(pool->queue_lock));
    /*���ˣ��ȴ��������������ˣ�����һ���ȴ��̣߳�
     ע����������̶߳���æµ�����û���κ�����*/
     pthread_cond_signal (&(pool->queue_ready));
    return 0;
}


/*�����̳߳أ��ȴ������е����񲻻��ٱ�ִ�У������������е��̻߳�һֱ
����������������˳�*/
int
pool_destroy ()
{
    if (pool->shutdown)
        return -1;/*��ֹ���ε���*/
     pool->shutdown = 1;

    /*�������еȴ��̣߳��̳߳�Ҫ������*/
     pthread_cond_broadcast (&(pool->queue_ready));

    /*�����ȴ��߳��˳�������ͳɽ�ʬ��*/
    int i;
    for (i = 0; i < pool->max_thread_num; i++)
         pthread_join (pool->threadid[i], NULL);
     free (pool->threadid);

    /*���ٵȴ�����*/
     CThread_worker *head = NULL;
    while (pool->queue_head != NULL)
     {
         head = pool->queue_head;
         pool->queue_head = pool->queue_head->next;
         free (head);
     }
    /*���������ͻ�����Ҳ����������*/
     pthread_mutex_destroy(&(pool->queue_lock));
     pthread_cond_destroy(&(pool->queue_ready));
    
     free (pool);
    /*���ٺ�ָ���ÿ��Ǹ���ϰ��*/
     pool=NULL;
    return 0;
}


void *
thread_routine (void *arg)
{
     printf ("starting thread 0x%x\n", pthread_self ());
    while (1)
     {
         pthread_mutex_lock (&(pool->queue_lock));
        /*����ȴ�����Ϊ0���Ҳ������̳߳أ���������״̬; ע��
         pthread_cond_wait��һ��ԭ�Ӳ������ȴ�ǰ����������Ѻ�����*/
        while (pool->cur_queue_size == 0 && !pool->shutdown)
         {
             printf ("thread 0x%x is waiting\n", pthread_self ());
             pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
         }

        /*�̳߳�Ҫ������*/
        if (pool->shutdown)
         {
            /*����break,continue,return����ת��䣬ǧ��Ҫ�����Ƚ���*/
             pthread_mutex_unlock (&(pool->queue_lock));
             printf ("thread 0x%x will exit\n", pthread_self ());
             pthread_exit (NULL);
         }

         printf ("thread 0x%x is starting to work\n", pthread_self ());

        /*assert�ǵ��Եĺð���*/
         assert (pool->cur_queue_size != 0);
         assert (pool->queue_head != NULL);
        
        /*�ȴ����г��ȼ�ȥ1����ȡ�������е�ͷԪ��*/
         pool->cur_queue_size--;
         CThread_worker *worker = pool->queue_head;
         pool->queue_head = worker->next;
         pthread_mutex_unlock (&(pool->queue_lock));

        /*���ûص�������ִ������*/
         (*(worker->process)) (worker->arg);
         free (worker);
         worker = NULL;
     }
    /*��һ��Ӧ���ǲ��ɴ��*/
     pthread_exit (NULL);
}

    �����ǲ��Դ���
void *
myprocess (void *arg)
{
     printf ("threadid is 0x%x, working on task %d\n", pthread_self (),*(int *) arg);
     sleep (1);/*��Ϣһ�룬�ӳ������ִ��ʱ��*/
    return NULL;
}

int
main (int argc, char **argv)
{
     pool_init (3);/*�̳߳������������߳�*/
    
    /*���������Ͷ��10������*/
    int *workingnum = (int *) malloc (sizeof (int) * 10);
    int i;
    for (i = 0; i < 10; i++)
     {
         workingnum[i] = i;
         pool_add_worker (myprocess, &workingnum[i]);
     }
    /*�ȴ������������*/
     sleep (5);
    /*�����̳߳�*/
     pool_destroy ();

     free (workingnum);
    return 0;
}
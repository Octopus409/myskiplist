#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include "../skiplist.h"

#define NUM_THREADS 1
#define TEST_COUNT 100000

// 自定义类型。
class A{
public:
    int a;
    char b[10];
    double c;
    A(){
        a = 2;
        c = 3.0;
    }
    A(int t){
        a = t;
        c = t*2;
    }
    // 重载比较运算符
    bool operator<(const A& v)const{
        return this->a<v.a;
    }
    
};

// 重载流运算符
std::ostream& operator<<(std::ostream& out, const A& tmp){
    out << tmp.a << std::endl;
    return out;
}

Skiplist<A, int> skipList;

/*
void *insertElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT/NUM_THREADS; 
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.insert(rand() % TEST_COUNT, count); 
	}
    pthread_exit(NULL);
}

void *getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT/NUM_THREADS; 
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.find(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}
*/

// 随机插入、删除、查询
void *random_man(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT/NUM_THREADS;
    for(int count=0; count<tmp; ++count){
        int man = rand()%3;
        switch (man)
        {
        case 1:
            {
                skipList.insert( A(rand() % TEST_COUNT), count);
            }
            break;
        case 2:
            {
                skipList.erase( A(rand() % TEST_COUNT));
            }
        default:
            {
                skipList.find( A(rand() % TEST_COUNT) );
            }
            break;
        }
    }
}

int main() {
    srand (time(NULL));  
    {

        pthread_t threads[NUM_THREADS];
        int rc;
        int i;

        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, random_man, (void *)&i);

            if (rc) {
                std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "random elapsed:" << elapsed.count() << std::endl;
    }
    std::cout << skipList.size() << std::endl;
    // skipList.display();

    // {
    //     pthread_t threads[NUM_THREADS];
    //     int rc;
    //     int i;
    //     auto start = std::chrono::high_resolution_clock::now();

    //     for( i = 0; i < NUM_THREADS; i++ ) {
    //         std::cout << "main() : creating thread, " << i << std::endl;
    //         rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

    //         if (rc) {
    //             std::cout << "Error:unable to create thread," << rc << std::endl;
    //             exit(-1);
    //         }
    //     }

    //     void *ret;
    //     for( i = 0; i < NUM_THREADS; i++ ) {
    //         if (pthread_join(threads[i], &ret) !=0 )  {
    //             perror("pthread_create() error"); 
    //             exit(3);
    //         }
    //     }

    //     auto finish = std::chrono::high_resolution_clock::now(); 
    //     std::chrono::duration<double> elapsed = finish - start;
    //     std::cout << "get elapsed:" << elapsed.count() << std::endl;
    // }

    srand (time(NULL));  
    {

        auto start = std::chrono::high_resolution_clock::now();

        skipList.dump_file_binary("stress.txt");
        skipList.load_file("stress.txt");
        
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "落盘然后读取的时间花费:" << elapsed.count() << std::endl;
    }
    std::cout << skipList.size() << std::endl;

	pthread_exit(NULL);
    return 0;

}

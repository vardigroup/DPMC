#ifndef TIMERS_H_
#define TIMERS_H_

#include <ctime>
#include <chrono>
#include <iostream>
#include <sys/resource.h>
#include <sys/time.h>

static inline double cpuTimeTotal(void){
	struct rusage ru;
	getrusage(RUSAGE_SELF,&ru);
	//cout<<"User time: "<<ru.ru_utime.tv_sec<<" Sys time: "<<ru.ru_stime.tv_sec<<endl;
	return (double) ru.ru_utime.tv_sec+(double) ru.ru_utime.tv_usec/1000000.0 + (double) ru.ru_stime.tv_sec+(double) ru.ru_stime.tv_usec/1000000.0;
}

static inline void print_wall_time (void)
{
    auto time_now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(time_now);
    std::cout << std::ctime(&end_time);
}

static inline double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        std::cout<<"Wall time may be wrong"<<std::endl;//  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

static inline double get_time_id(void){
    auto time_now = std::chrono::high_resolution_clock::now();
    double nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(time_now.time_since_epoch()).count();
    return nanos;
}

#endif

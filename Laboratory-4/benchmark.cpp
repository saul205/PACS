#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <threadsafe_queue_lockFree.hpp>
#include <threadsafe_queue.hpp>
#include <fstream>
#include <math.h>

using namespace std;

template<typename T>
void worker_push(T& queue, int N, int N2){

    for(int i = N; i < N2; i++){
        queue.push(i);
    }
}
template<typename T>
void worker_pop(T& queue, int N, int N2){

    int buffer;
    for(int i = N; i < N2; i++){
        while(!queue.try_pop(buffer));
    }
}
template<typename T>
void prepare_benchmark_pop(T& queue, int N, size_t threads, int block_size){
    std::vector<std::thread> thread_vector;
    for(size_t i = 0; i < threads; i++){
        thread_vector.push_back(
            std::thread(worker_push<T>, std::ref(queue), i*block_size, std::min((int)(i+1)*block_size, N))
        );
    }

    for(size_t i = 0; i < threads; i++){
        thread_vector[i].join();
    }

    queue.print(true);
}
template<typename T>
void push_benchmark(std::vector<std::thread>&thread_vector, int N, size_t threads, T& queue, int block_size){

    for(size_t i = 0; i < threads; i++){

        thread_vector.push_back(
                    std::thread(worker_push<T>, std::ref(queue), i*block_size, std::min((int)(i+1)*block_size, N))
                );
    }
}
template<typename T>
void pop_benchmark(std::vector<std::thread>& thread_vector, int N, size_t threads, T& queue, int block_size){
    
    for(size_t i = 0; i < threads; i++){

        thread_vector.push_back(
                    std::thread(worker_pop<T>, std::ref(queue), i*block_size, std::min((int)(i+1)*block_size, N))
                );
    }
}
template<typename T>
void push_pop_benchmark(std::vector<std::thread>& thread_vector, int N, size_t threads, T& queue, int block_size){

    for(size_t i = 0; i < threads; i++){

        thread_vector.push_back(
                    std::thread(worker_push<T>, std::ref(queue), i*block_size, std::min((int)(i+1)*block_size, N))
                );
        thread_vector.push_back(
                    std::thread(worker_pop<T>, std::ref(queue), i*block_size, std::min((int)(i+1)*block_size, N))
                );
    }
}

template<typename T>
int64_t perform_benchmark(int N, size_t threads, int bench, string useLocks){

    T queue;
    auto function = push_benchmark<T>;

    int block_size = N / threads;
    int res = N - block_size * threads > 0;
    if(res > 0)
        block_size += res; 

    string name = "";
    switch (bench)
    {
    case 2:
        prepare_benchmark_pop(queue, N, threads, block_size);
        function = pop_benchmark;
        name = "pop";
        break;
    case 3:
        function = push_pop_benchmark;
        name = "push / pop";
        break;
    default:
        function = push_benchmark;
        name = "push";
        break;
    }

    std::vector<std::thread> thread_vector;
    // please complete missing parts
    auto start = std::chrono::steady_clock::now();

    function(thread_vector, N, threads, queue, block_size);

    cout << thread_vector.size() << endl;
    for(size_t i = 0; i < thread_vector.size(); i++){
        thread_vector[i].join();
    }

    auto stop = std::chrono::steady_clock::now();
    int64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
    std::cout << "Benchmark " + name + useLocks << time << " ms." << std::endl;

    queue.print(true);
    return time;
}

int parse_params(int argc, const char *argv[], int& N, int& bench, size_t& threads, bool &complete, bool &lock){
    for(int i = 1; i < argc; i++){
        if(string(argv[i]) == "-h" || string(argv[i]) == "--help"){
           std::cout << "Usage: ./benchmarks [-h/--help <Print help>] [-n <Operations per thread>] [-t <Number of threads>] [-b <Benchmark>]}" << std::endl;
           std::cout << "  -n: Number of pushes or pops. Default: 100" << endl; 
           std::cout << "  -t: Number of threads, please an even value is needed for benchmark 3. Default: 1" << endl; 
           std::cout << "  -b: Benchmark to execute. Default: 1" << endl; 
           std::cout << "       - 1 : push" << endl; 
           std::cout << "       - 2 : pop" << endl; 
           std::cout << "       - 3 : push & pop at the same time" << endl; 
           std::cout << " - locks : Use queue with locks" << endl; 
           std::cout << " - c : Complete benchmark" << endl; 
           std::cout << "  -h/--help : Print help" << endl; 

           return 1;
        }else if(string(argv[i]) == "-n"){
            N = stoi(argv[++i]);
        }
        else if(string(argv[i]) == "-t"){
            threads = stoi(argv[++i]);
        }
        else if(string(argv[i]) == "-b"){
            bench = stoi(argv[++i]);
        }
        else if(string(argv[i]) == "-l"){
            lock = true;
        }
        else if(string(argv[i]) == "-c"){
            complete = true;
        }   
        else{
            return -1;
        }
    }
    return 0;
}

int64_t execute_for_type(bool lock, int N, size_t threads, int bench){
    if(lock){
        return perform_benchmark<threadsafe_queue<int>>(N, threads, bench, " with locks: ");
    }
    else{
        auto a = perform_benchmark<threadsafe_queue_lockFree<int>>(N, threads, bench, " without locks: ");
        return a;
    }

    return 0;
}

int main(int argc, const char *argv[]){

    int N = 100, bench = 1;
    size_t threads = 1;
    bool complete = false, lock = false;

    int parse = parse_params(argc, argv, N, bench, threads, complete, lock);
    if( parse == -1){
        std::cout << "Usage: ./benchmarks [-h/--help <Print help>] [-n <Operations per thread>] [-t <Number of threads>] [-b <Benchmark>]}" << std::endl;
        return -1;
    }else if(parse == 1){
        return 0;
    }

    if(complete){

        std::vector<std::vector<std::vector<int64_t>>> times;

        for(int j = 1; j < 4; j++){
            std::vector<std::vector<int64_t>> times_lock;
            for(int b = 0; b < 3; b++){
                bool useLocks = b % 2 == 0;
                std::vector<int64_t> times_aux;
                for(int i = 0; i < 10; i++){
                    times_aux.push_back(execute_for_type(useLocks, N, threads, j));
                }
                times_lock.push_back(times_aux);
            }
            times.push_back(times_lock);
        }

        ofstream output("results_" + std::to_string(N) + "_" + std::to_string(threads) + ".txt");

        for(int i = 1; i < 4; ++i){
            for(int j=0; j < 2; ++j){
                float mean = 0, std = 0;
                if(j % 2 == 0) 
                    output << "With locks: "; 
                else
                    output << "Lock free: ";
                for(float time : times[i-1][j]){
                    output << time << " "; 
                    mean += time;
                }
                mean /= times[i-1][j].size();
                for(float time : times[i-1][j]){
                    std += (time - mean) * (time - mean);
                }

                std = sqrt(std / times.size());
                float coef = mean == 0 ? 0 : std/mean;
                output << std::endl << std::to_string(mean) << " " << std::to_string(std) << " " << std::to_string(coef) << endl << endl;
            }
        }

        output.close();
    }
    else
        execute_for_type(lock, N, threads, bench);

    return 0;
}
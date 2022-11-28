#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <threadsafe_queue_lockFree.hpp>

using namespace std;

void worker_push(threadsafe_queue_lockFree<int>& queue, int N, int N2){

    for(int i = N; i < N2; i++){
        queue.push(i);
    }
}

void worker_pop(threadsafe_queue_lockFree<int>& queue, int N, int N2){

    int buffer;
    for(int i = N; i < N2; i++){
        while(!queue.try_pop(buffer));
    }
}

void prepare_benchmark_pop(threadsafe_queue_lockFree<int>& queue, int N, size_t threads){
    std::vector<std::thread> thread_vector;
    for(size_t i = 0; i < threads; i++){
        thread_vector.push_back(
            std::thread(worker_push, std::ref(queue), i*N, (i+1)*N)
        );
    }

    for(size_t i = 0; i < threads; i++){
        thread_vector[i].join();
    }

    queue.print(true);
}
void push_benchmark(std::vector<std::thread>&thread_vector, int N, size_t threads, threadsafe_queue_lockFree<int>& queue){

    for(size_t i = 0; i < threads; i++){

        thread_vector.push_back(
                    std::thread(worker_push, std::ref(queue), i*N, (i+1)*N)
                );
    }
}
void pop_benchmark(std::vector<std::thread>& thread_vector, int N, size_t threads, threadsafe_queue_lockFree<int>& queue){
    
    for(size_t i = 0; i < threads; i++){

        thread_vector.push_back(
                    std::thread(worker_pop, std::ref(queue), i*N, (i+1)*N)
                );
    }
}
void push_pop_benchmark(std::vector<std::thread>& thread_vector, int N, size_t threads, threadsafe_queue_lockFree<int>& queue){

    for(size_t i = 0; i < threads; i++){

        thread_vector.push_back(
                    std::thread(worker_push, std::ref(queue), i*N, (i+1)*N)
                );
        thread_vector.push_back(
                    std::thread(worker_pop, std::ref(queue), i*N/2, (i+1)*N/2)
                );
    }
}

void perform_benchmark(int N, size_t threads, int bench){

    threadsafe_queue_lockFree<int> queue;
    auto function = push_benchmark;
    switch (bench)
    {
    case 2:
        prepare_benchmark_pop(queue, N, threads);
        function = pop_benchmark;
        break;
    case 3:
        function = push_pop_benchmark;
        break;
    default:
        function = push_benchmark;
        break;
    }

    std::vector<std::thread> thread_vector;
    // please complete missing parts
    auto start = std::chrono::steady_clock::now();

    function(thread_vector, N, threads, queue);

    cout << thread_vector.size() << endl;
    for(size_t i = 0; i < thread_vector.size(); i++){
        thread_vector[i].join();
    }

    auto stop = std::chrono::steady_clock::now();
    std::cout << "Benchmark loads with locks: " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << " ms." << std::endl;

    queue.print(true);
}

int parse_params(int argc, const char *argv[], int& N, int& bench, size_t& threads){
    for(int i = 1; i < argc; i++){
        if(string(argv[i]) == "-h" || string(argv[i]) == "--help"){
           std::cout << "Usage: ./benchmarks [-h/--help <Print help>] [-n <Operations per thread>] [-t <Number of threads>] [-b <Benchmark>]}" << std::endl;
           std::cout << "  -n: Number of pushes or pops per thread. Default: 100" << endl; 
           std::cout << "  -t: Number of threads, please an even value is needed for benchmark 3. Default: 1" << endl; 
           std::cout << "  -b: Benchmark to execute. Default: 1" << endl; 
           std::cout << "       - 1 : push" << endl; 
           std::cout << "       - 2 : pop" << endl; 
           std::cout << "       - 3 : push & pop at the same time" << endl; 
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
        else{
            return -1;
        }
    }
    return 0;
}

int main(int argc, const char *argv[]){

    int N = 100, bench = 1;
    size_t threads = 1;

    int parse = parse_params(argc, argv, N, bench, threads);
    if( parse == -1){
        std::cout << "Usage: ./benchmarks [-h/--help <Print help>] [-n <Operations per thread>] [-t <Number of threads>] [-b <Benchmark>]}" << std::endl;
        return -1;
    }else if(parse == 1){
        return 0;
    }

    perform_benchmark(N, threads, bench);

    return 0;
}
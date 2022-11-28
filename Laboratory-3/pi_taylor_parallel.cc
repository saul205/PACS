#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <fstream>
#include <cmath>

using my_float = long double;

void
pi_taylor_chunk(std::vector<my_float>& output,
    const size_t& thread_id, const size_t& start_step, const size_t& stop_step) {

    my_float res = 0.;
    int sign = start_step % 2 == 0 ? 1 : -1;
    for (size_t i = start_step; i < stop_step; i++) {
        res += sign / (1.l + 2.l * i);
        sign = -sign;
    }

    output[thread_id] = 4. * res;
}

std::tuple<size_t, size_t, int>
usage(int argc, const char* argv[]) {
    // read the number of steps from the command line
    if (argc != 4) {
        std::cerr << "Invalid syntax: pi_taylor <steps> <threads> <n_times>" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    size_t threads = std::stoll(argv[2]);
    int times = std::stoi(argv[3]);

    if (steps < threads) {
        std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
        exit(1);

    }
    return std::make_tuple(steps, threads, times);
}

int main(int argc, const char* argv[]) {

    auto ret_pair = usage(argc, argv);
    auto steps = std::get<0>(ret_pair);
    auto threads = std::get<1>(ret_pair);
    int n_times = std::get<2>(ret_pair);

    my_float pi;
    
    
    size_t chunk_size = steps / threads;
    // please complete missing parts

    std::vector<my_float> times;

    for(int t = 0; t < n_times; ++t){
        
        std::vector<my_float> results(threads);
        std::vector<std::thread> thread_vector;

        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < threads - 1; i++) {
            thread_vector.push_back(
                std::thread(pi_taylor_chunk, std::ref(results),
                    i, i * chunk_size, i * chunk_size + chunk_size)
            );
        }

        pi_taylor_chunk(std::ref(results), threads - 1, (threads - 1) * chunk_size, steps-1);

        for (size_t i = 0; i < threads - 1; i++) {
            thread_vector[i].join();
        }
        auto stop = std::chrono::steady_clock::now();

        pi = std::accumulate(results.begin(), results.end(), (my_float)0);

        std::chrono::duration<my_float> duration = stop - start;

        std::cout << "For " << steps << ", pi value: "
            << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
            << pi << std::endl;
        std::cout << "Elapsed time: " << duration.count() << std::endl;

        times.push_back(duration.count());
    }

    std::cout << "Mean Elapsed time: " << std::accumulate(times.begin(), times.end(), (my_float)0) / (my_float)n_times << std::endl;

    std::fstream output("results_lab_3_parallel.txt", std::fstream::app);
    output << "N_steps: " << steps << ", Threads: " << threads << std::endl;
    my_float mean = std::accumulate(times.begin(), times.end(), (my_float)0) / (my_float)n_times;
    my_float std = 0;
    for(float time : times){
        output << time << " "; 
        std += (time - mean) * (time - mean);
    }

    std = sqrt(std / times.size());
    
    output << std::endl << std::to_string(mean) << " " << std::to_string(std) << " " << std::to_string(std/mean);
    output << std::endl << std::endl;
}

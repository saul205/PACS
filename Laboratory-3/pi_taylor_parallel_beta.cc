#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using my_float = long double;

void
pi_taylor_chunk(std::vector<my_float>& output, std::vector<my_float>& times,
    const size_t& thread_id, const size_t& start_step, const size_t& stop_step) {
    
    auto start = std::chrono::steady_clock::now();
    my_float res = 0.;
    int sign = start_step % 2 == 0 ? 1 : -1;
    for (size_t i = start_step; i < stop_step; i++) {
        res += sign / (1.l + 2.l * i);
        sign = -sign;
    }
    output[thread_id] = 4. * res; 
    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<my_float> duration = stop - start;
    times[thread_id] = duration.count();
}

std::pair<size_t, size_t>
usage(int argc, const char* argv[]) {
    // read the number of steps from the command line
    if (argc != 3) {
        std::cerr << "Invalid syntax: pi_taylor <steps> <threads>" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    size_t threads = std::stoll(argv[2]);

    if (steps < threads) {
        std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
        exit(1);

    }
    return std::make_pair(steps, threads);
}

int main(int argc, const char* argv[]) {


    auto ret_pair = usage(argc, argv);
    auto steps = ret_pair.first;
    auto threads = ret_pair.second;

    my_float pi;
    std::vector<my_float> results(threads);
    std::vector<my_float> times(threads);
    std::vector<std::thread> thread_vector;
    auto chunk_size = steps / threads;
    // please complete missing parts

    auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < threads - 1; i++) {
        thread_vector.push_back(
            std::thread(pi_taylor_chunk, std::ref(results), std::ref(times),
                i, i * chunk_size, i * chunk_size + chunk_size)
        );
    }


    pi_taylor_chunk(std::ref(results), std::ref(times), threads - 1, (threads - 1) * chunk_size, (threads - 1) * chunk_size + chunk_size);


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

    for(size_t i = 0; i < times.size(); i++){
        std::cout << "Thread " << i << ": " << times[i] << std::endl;
    }
}

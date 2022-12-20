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

// Allow to change the floating point type
using my_float = long double;

my_float pi_taylor(size_t steps) {

    my_float res = 0.;
    int sign = 1;
    for(size_t i = 0; i < steps; i++){
        res += sign / (1.l + 2.l*i);
        sign = -sign;
    }

    return 4.*res;
}

int main(int argc, const char *argv[]) {

    int nTimes {1};
    // read the number of steps from the command line
    if (argc < 2 || argc > 3) {
        std::cerr << "Invalid syntax: pi_taylor <steps> [<nTimes>]" << std::endl;
        exit(1);

    }

    nTimes = std::stoi(argv[2]);
    std::vector<my_float> times;
    size_t steps = std::stoll(argv[1]);

    for(int i = 0; i < nTimes; i++){
        auto now {std::chrono::system_clock::now()};
        auto pi = pi_taylor(steps);
        
        auto then = std::chrono::system_clock::now();
        std::chrono::duration<my_float> duration {then - now};
        times.push_back(duration.count());

        std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<my_float>::digits10 + 1)
        << pi << std::endl;
    }

    std::cout << "Mean Elapsed time: " << std::accumulate(times.begin(), times.end(), (my_float)0) / (my_float)nTimes << std::endl;

    std::fstream output("results_lab_3_sequential.txt", std::fstream::app);
    output << "N_steps: " << steps << std::endl;
    my_float mean = std::accumulate(times.begin(), times.end(), (my_float)0) / (my_float)nTimes;
    my_float std = 0;
    for(float time : times){
        output << time << " "; 
        std += (time - mean) * (time - mean);
    }

    std = sqrt(std / times.size());
    
    output << std::endl << std::to_string(mean) << " " << std::to_string(std) << " " << std::to_string(std/mean);
    output << std::endl << std::endl;
}

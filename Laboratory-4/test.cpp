#include <memory>
#include <atomic>
int main(){
    std::atomic<std::shared_ptr<int>> point{std::make_shared<int>(1)};

    return 0;
}
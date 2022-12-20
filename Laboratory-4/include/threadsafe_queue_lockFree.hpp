#pragma once

#include <memory>
#include <atomic>
#include <iostream>

template<typename T>
class threadsafe_queue_lockFree
{
  private:
    
      struct node_t{
        T value;
        std::atomic<std::shared_ptr<node_t>> next;
      };

    std::atomic<std::shared_ptr<node_t>> head, tail;

  public:
    
    threadsafe_queue_lockFree() {
        std::shared_ptr<node_t> node = std::make_shared<node_t>();
        node->next.store(nullptr);
        head.store(node);
        tail.store(node);
    }

    threadsafe_queue_lockFree(const threadsafe_queue_lockFree& other)
    {
        head.store(other.head.load());
        tail.store(other.tail.load());
    }

    ~threadsafe_queue_lockFree(){

        std::shared_ptr<node_t> node = head.load();
        std::shared_ptr<node_t> killer;
        head.store(nullptr);
        tail.store(nullptr);
        while(node != nullptr){
            
            std::swap(killer, node);
            node = killer->next.load();
            killer.reset();
        }
    }

    threadsafe_queue_lockFree& operator=(const threadsafe_queue_lockFree&) = delete;

    void push(const T& new_value)
    {
        std::shared_ptr<node_t> node_aux(new node_t());
        node_aux->value = new_value;
        node_aux->next.store(nullptr);
        std::atomic<std::shared_ptr<node_t>> node = node_aux;
        std::shared_ptr<node_t> cola;
        for(;;){
            cola = tail.load();
            std::shared_ptr<node_t> next = cola->next.load();

            if(cola == tail.load()){
                if(next == nullptr){
                    if(cola->next.compare_exchange_weak(next, node)){
                        break;
                    }
                }else
                    tail.compare_exchange_weak(cola, next);
            }
        }
        tail.compare_exchange_weak(cola, node);
    }

    bool try_pop(T& value)
    {
        std::shared_ptr<node_t> head_aux(new node_t()); 
        for(;;){
            head_aux = head.load();
            std::shared_ptr<node_t> tail_aux = tail.load();
            std::shared_ptr<node_t> next = head_aux->next.load();

            if(head_aux == head.load()){
                if(head_aux == tail_aux){
                    if(next == nullptr){
                        return false;
                    }
                    tail.compare_exchange_weak(tail_aux, next);
                }else{
                    value = next->value;
                    if(head.compare_exchange_weak(head_aux, next)){
                        
                        break;
                    }
                }
            }
        }
        head_aux.reset();
        return true;
    }   

    bool empty() const
    {   
        std::shared_ptr<node_t> head_aux = head.load();
        return head_aux->next.load() == nullptr;
    }

    void print(bool count){

        int c = 0;
        std::shared_ptr<node_t> node = head.load();
        while(node->next.load() != nullptr){
            if(!count) std::cout << node->next.load()->value << " ";

            node = node->next.load();
            c++;
        }
        std::cout << (count ? std::to_string(c) : "") << std::endl;
    }
};

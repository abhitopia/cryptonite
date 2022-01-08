//
// Created by Abhishek Aggarwal on 02/01/2022.
//

#ifndef CRYPTONITE_TOP_N_CONTAINER_H
#define CRYPTONITE_TOP_N_CONTAINER_H

#include <queue>
#include <vector>

template <typename T>
struct TopNCompare {
    bool operator()(const T &x, const T &y){
        return x > y;
    }
};


template <typename T>
class TopNContainer {
    int _maxSize{};
    std::priority_queue<T, std::vector<T>, TopNCompare<T>> _pq;

public:
    TopNContainer(int maxSize){
        _maxSize = maxSize;
        _pq = std::priority_queue<T, std::vector<T>, TopNCompare<T>>();
    }

    bool insert(const T& val){
        if(_pq.size() < _maxSize)
        {
            _pq.push(val);
            return true;
        }
        if(val > _pq.top())
        {
            _pq.pop(); //get rid of the root
            _pq.push(val); //priority queue will automatically restructure
            return true;
        }
        return false;
    }

    std::vector<T> getSorted(){
        std::priority_queue<T,std::vector<T>, TopNCompare<T>> cp= _pq;
        std::vector<T> result{};
        result.resize(cp.size());
        for(int i=cp.size()-1; i>=0; i--){
            result[i] = cp.top();
            cp.pop();
        }
        return result;
    }

    int size() const {
        return _pq.size();
    }
};

#endif //CRYPTONITE_TOP_N_CONTAINER_H

#pragma once
/**
 * \file
 * \brief
 * \author Lectem
 */
#include <queue>
#include <vector>

template <typename T, std::size_t ChunkSize = 8192>
class BasicPool
{
public:
    class iterator;

    explicit BasicPool()
        : end_(0), element_size_(sizeof(T)), chunk_size_(ChunkSize) {reserve(1);}

    ~BasicPool()
    {
        clear();
        for (char *ptr : blocks_)delete[] ptr;
        for (bool *ptr : blocks_usage_)delete[] ptr;
    }

    template<class ...Args>
    iterator emplace(Args ...args)
    {
        iterator it(this,getAvailableIndex());
        new(get(it)) T(args...);
        usage(it)=true;
        ++size_;
        return it;
    }

    void remove(iterator it)
    {
        bool& usageit=usage(it);
        if(usageit)
        {
            get(it)->~T();
            usageit=false;
            free_slots_indexes_.push(it);
            --size_;
        }
    }

    void clear()
    {
        for(iterator it=begin(); it<end(); ++it)
        {
            bool& usageit=usage(it);
            assert(usageit);
            get(it)->~T();
            usageit=false;
        }
        std::queue<std::size_t> emptyqueue;
        free_slots_indexes_.swap(emptyqueue);
        size_=0;
        end_=0;
    }

    inline iterator begin()
    {
        std::size_t tmp=0;
        while(!usage(tmp) && tmp<end_)tmp++;
        return iterator(this,tmp);
    }
    inline iterator end()
    {
        return iterator(this,end_);
    }

    std::size_t size(){return size_;}

protected:
    //INTERNAL USAGE ONLY
    inline void reserve(std::size_t n)
    {
        while (capacity_ < n)
        {
            char *chunk = new char[element_size_ * chunk_size_];
            blocks_.push_back(chunk);
            bool *chunk_usage = new bool[chunk_size_];
            blocks_usage_.push_back(chunk_usage);
            capacity_ += chunk_size_;
        }
    }


    inline std::size_t getAvailableIndex()
    {
        if(end_ < capacity_)return end_++;
        else if(free_slots_indexes_.size()>0)
        {
            size_t index = free_slots_indexes_.front();
            free_slots_indexes_.pop();
            return index;
        }
        else
        {
            reserve(capacity_+chunk_size_);
            return end_++;
        }
    }

    inline T *get(std::size_t n)
    {
        assert(n < capacity_);
        return (T*) (blocks_[n / chunk_size_] + (n % chunk_size_) * element_size_);
    }

    inline const T *get(std::size_t n) const
    {
        assert(n < capacity_);
        return blocks_[n / chunk_size_] + (n % chunk_size_) * element_size_;
    }

    inline bool& usage(std::size_t n)
    {
        assert(n < capacity_);
        return *(blocks_usage_[n / chunk_size_] + (n % chunk_size_) * sizeof(bool));
    }

    inline bool& usage(std::size_t n) const
    {
        assert(n < capacity_);
        return *(blocks_usage_[n / chunk_size_] + (n % chunk_size_) * sizeof(bool));
    }

    std::vector<char*> blocks_;
    std::vector<bool*> blocks_usage_;
    std::queue<size_t> free_slots_indexes_;
    std::size_t end_;
    std::size_t element_size_;
    std::size_t chunk_size_;
    std::size_t size_ = 0;
    std::size_t capacity_;
};

template <typename T, std::size_t ChunkSize = 8192>
class BasicPool<T,ChunkSize>::iterator: public std::iterator<std::input_iterator_tag, T>
{
    std::size_t ind_;
    BasicPool<T,ChunkSize> *pool_;
public:
    iterator(BasicPool<T,ChunkSize> *pool,std::size_t x) :ind_(x),pool_(pool) {}
    iterator(const iterator& it) : ind_(it.ind_),pool_(it.pool_) {}
    iterator& operator++() {while(ind_ < pool_->end_ && !pool_->usage(ind_)){ind_++;};return *this;}
    iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
    bool operator==(const iterator& rhs) {return ind_==rhs.ind_ && pool_==rhs.pool_;}
    bool operator!=(const iterator& rhs) {return ind_!=rhs.ind_ || pool_!=rhs.pool_;}
    T& operator*() {return *(pool_->get(ind_));}
    operator std::size_t (){return ind_;}
};

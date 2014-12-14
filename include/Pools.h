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
        : element_size_(sizeof(T)), chunk_size_(ChunkSize)
    {reserve(1);}

    ~BasicPool()
    {
        clear();
        for (char *ptr : blocks_)delete[] ptr;

    }

    template<class ...Args>
    iterator emplace(Args &&...args)
    {
        iterator it(this,getAvailableIndex());
        new(get(it)) T(std::forward<Args>(args)...);
        usage(it)=true;
        ++size_;
        return it;
    }

    void remove(iterator it)
    {
        vector<bool>::reference usageit=usage(it);
        if(usageit)
        {
            get(it)->~T();
            usageit=false;
            free_slots_indexes_.push(it.ind_);
            if(!(--size_))end_=0;
        }
    }

    void clear()
    {
        for(iterator it=begin(); it != end(); ++it)
        {
            vector<bool>::reference usageit=usage(it);
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
        while(begin_<end_ && !usage(begin_) )begin_++;
        return iterator(this,begin_);
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
            capacity_ += chunk_size_;
            blocks_usage_.resize(capacity_);
        }
    }


    inline std::size_t getAvailableIndex()
    {
        if(end_ < capacity_)return end_++;
        else if(free_slots_indexes_.size()>0)
        {
            size_t index = free_slots_indexes_.front();
            free_slots_indexes_.pop();
            if(index<begin_)begin_=index;
            return index;
        }
        else
        {
            reserve(capacity_+chunk_size_);
            assert(end_ < capacity_);
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

    inline T *get(iterator it)
    {
        return get(it.ind_);
    }

    inline const T *get(iterator it) const
    {
        return get(it.ind_);
    }

    inline vector<bool>::reference usage(std::size_t n)
    {
        assert(n < capacity_);
        return blocks_usage_[n];
    }

    inline vector<bool>::reference usage(std::size_t n) const
    {
        assert(n < capacity_);
        return blocks_usage_[n];
    }
    inline vector<bool>::reference usage(iterator it)
    {
        return usage(it.ind_);
    }

    inline vector<bool>::reference usage(iterator it) const
    {
        return usage(it.ind_);
    }

    std::vector<char*> blocks_;
    std::vector<bool> blocks_usage_;
    std::queue<size_t> free_slots_indexes_;
    std::size_t begin_=0;
    std::size_t end_=0;
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
    iterator& operator++() {while(ind_ < pool_->end_ && !pool_->usage(ind_)){++ind_;}return *this;}
    iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
    bool operator==(const iterator& rhs) {return ind_==rhs.ind_ && pool_==rhs.pool_;}
    bool operator!=(const iterator& rhs) {return ind_!=rhs.ind_ || pool_!=rhs.pool_;}
    T& operator*() {return *(pool_->get(ind_));}
    friend BasicPool<T,ChunkSize>;
};

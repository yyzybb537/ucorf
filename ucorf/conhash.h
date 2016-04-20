#pragma once

#include <unordered_map>
#include <map>
#include <string>

namespace ucorf
{

template <typename V, template <typename> class H = std::hash>
    class con_hashtable
    {
    public:
        typedef V value_t;

        template <typename T>
        std::size_t insert(T const& key, value_t value, std::size_t vir_count = 64)
        {
            std::size_t ori_hashcode = hash(key);
            if (vir_table_.count(ori_hashcode)) return 0;
            auto &vir_list = vir_table_[ori_hashcode];

            std::string ori_str = std::to_string(ori_hashcode);
            for (std::size_t i = 0; i < vir_count; ++i)
            {
                std::string str = ori_str + "-" + std::to_string(i);
                std::size_t hashcode = hash(str);
                if (table_.insert(std::make_pair(hashcode, value)).second)
                    vir_list.push_back(hashcode);
            }

            if (vir_list.empty()) {
                vir_table_.erase(ori_hashcode);
                return 0;
            }

            return vir_list.size();
        }

        template <typename T>
        bool erase(T const& key)
        {
            std::size_t ori_hashcode = hash(key);
            auto it = vir_table_.find(ori_hashcode);
            if (vir_table_.end() == it) return false;
            auto &vir_list = it->second;
            for (auto hashcode : vir_list)
                table_.erase(hashcode);
            vir_table_.erase(it);
            return true;
        }

        template <typename T>
        value_t get(T const& key)
        {
            return hget(hash(key));
        }

        value_t hget(std::size_t hash_code)
        {
            if (table_.empty()) return value_t();

            auto it = table_.lower_bound(hash_code);
            if (it == table_.end())
                it = table_.begin();

            return it->second;
        }

    private:
        template <typename T>
        std::size_t hash(T const& key)
        {
            return H<T>()(key);
        }

    private:
        typedef std::map<std::size_t, V> table_t;
        typedef std::map<std::size_t, std::vector<std::size_t> > vir_table_t;
        table_t table_;
        vir_table_t vir_table_;
    };


} //namespace ucorf

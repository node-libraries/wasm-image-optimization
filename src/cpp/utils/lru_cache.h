#ifndef IMAGE_CONVERTER_LRU_CACHE_H
#define IMAGE_CONVERTER_LRU_CACHE_H

#include <list>
#include <unordered_map>

namespace image_converter {

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class LruCache {
   public:
    typedef typename std::pair<Key, Value> key_value_pair_t;
    typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

    LruCache(size_t max_size) : m_max_size(max_size) {}

    void put(const Key& key, const Value& value) {
        auto it = m_cache_items_map.find(key);
        m_cache_items_list.push_front(key_value_pair_t(key, value));
        if (it != m_cache_items_map.end()) {
            m_cache_items_list.erase(it->second);
            m_cache_items_map.erase(it);
        }
        m_cache_items_map[key] = m_cache_items_list.begin();

        if (m_cache_items_map.size() > m_max_size) {
            auto last = m_cache_items_list.end();
            last--;
            m_cache_items_map.erase(last->first);
            m_cache_items_list.pop_back();
        }
    }

    Value* get(const Key& key) {
        auto it = m_cache_items_map.find(key);
        if (it == m_cache_items_map.end()) {
            return nullptr;
        } else {
            m_cache_items_list.splice(m_cache_items_list.begin(), m_cache_items_list, it->second);
            return &it->second->second;
        }
    }

    bool exists(const Key& key) const {
        return m_cache_items_map.find(key) != m_cache_items_map.end();
    }

    size_t size() const { return m_cache_items_map.size(); }

    void clear() {
        m_cache_items_map.clear();
        m_cache_items_list.clear();
    }

   private:
    std::list<key_value_pair_t> m_cache_items_list;
    std::unordered_map<Key, list_iterator_t, Hash> m_cache_items_map;
    size_t m_max_size;
};

}  // namespace image_converter

#endif // IMAGE_CONVERTER_LRU_CACHE_H

#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
#include <mutex>

template <typename T>
static std::vector<T> extend(std::vector<T> base, T element)
{
    base.push_back(element);
    return base;
}

namespace llpp::config
{
    inline std::mutex save_mutex;
    using json = nlohmann::ordered_json;

    json& get_data();

    template <typename T>
    struct ManagedVar
    {
    public:
        explicit ManagedVar(const std::vector<std::string>& t_var_path,
                            const std::function<void()>& t_on_change,
                            T t_default) : path_(t_var_path), on_change_(t_on_change)
        {
            try { get(); }
            catch (const json::out_of_range& e) {
                std::cout << "[!] Var " << t_var_path.back() << " does not yet exist!\n";
                set(t_default);
            }
        }

        explicit ManagedVar(const std::vector<std::string>& t_base, std::string t_key,
                            const std::function<void()>& t_on_change,
                            T t_default) : ManagedVar(extend(t_base, std::move(t_key)),
                                                      t_on_change, t_default) {}

        ~ManagedVar() = default;

        T get();
        T* get_ptr() { return &value_; }
        void set(const T& value);
        void save();

    private :
        T value_;
        std::vector<std::string> path_;
        std::function<void()> on_change_;

        json& walk_json(json& data, bool create_if_not_exist = false) const;
    };
}
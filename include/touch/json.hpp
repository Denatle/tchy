#pragma once
#include <json-c/json.h>
#include <memory>
#include <stdexcept>
#include <string>
namespace touch {
class Json {
    std::shared_ptr<json_object> p;
    explicit Json(json_object* o) : p(o, json_object_put) {}

  public:
    Json() : Json(json_object_new_object()) {}
    Json(const Json& other) {
        if (other.p) {
            json_object* clone = nullptr;
            if (json_object_deep_copy(other.p.get(), &clone, nullptr) != 0)
                throw std::runtime_error("Cannot copy JSON value");
            p = std::shared_ptr<json_object>(clone, json_object_put);
        }
    }
    Json& operator=(const Json& other) {
        if (this != &other) {
            Json copy(other);
            p.swap(copy.p);
        }
        return *this;
    }
    Json(Json&&) noexcept = default;
    Json& operator=(Json&&) noexcept = default;
    Json(const char* s) : Json(json_object_new_string(s)) {}
    Json(const std::string& s) : Json(s.c_str()) {}
    Json(double n) : Json(json_object_new_double(n)) {}
    Json(int n) : Json(json_object_new_int(n)) {}
    Json(uint64_t n) : Json(json_object_new_uint64(n)) {}
    static Json array() {
        return Json(json_object_new_array());
    }
    static Json parse(const std::string& s) {
        auto t = json_tokener_new();
        auto o = json_tokener_parse_ex(t, s.data(), int(s.size()));
        auto e = json_tokener_get_error(t);
        json_tokener_free(t);
        if (e != json_tokener_success) {
            if (o)
                json_object_put(o);
            throw std::runtime_error("Invalid JSON");
        }
        return Json(o);
    }
    bool has(const char* k) const {
        json_object* v;
        return json_object_object_get_ex(p.get(), k, &v);
    }
    Json operator[](const char* k) const {
        json_object* v = nullptr;
        json_object_object_get_ex(p.get(), k, &v);
        return Json(json_object_get(v));
    }
    Json operator[](size_t i) const {
        return Json(json_object_get(json_object_array_get_idx(p.get(), i)));
    }
    void set(const char* k, const Json& v) {
        json_object_object_add(p.get(), k, json_object_get(v.p.get()));
    }
    void erase(const char* key) {
        json_object_object_del(p.get(), key);
    }
    void push(const Json& v) {
        json_object_array_add(p.get(), json_object_get(v.p.get()));
    }
    size_t size() const {
        return json_object_is_type(p.get(), json_type_array) ? json_object_array_length(p.get())
                                                             : 0;
    }
    std::string str() const {
        auto s = json_object_get_string(p.get());
        return s ? s : "";
    }
    double number() const {
        return json_object_get_double(p.get());
    }
    uint64_t id() const {
        return json_object_get_uint64(p.get());
    }
    std::string dump() const {
        return json_object_to_json_string_ext(p.get(), JSON_C_TO_STRING_PRETTY);
    }
    bool operator==(const Json& other)const{return json_object_equal(p.get(),other.p.get());}
    bool null() const {
        return !p;
    }
};
} // namespace touch

#ifndef BASE_SKETCH_H
#define BASE_SKETCH_H

#include <string>

namespace sketch {

class BaseSketch {
public:
    virtual void Insert(const std::string &str) = 0;
    virtual std::pair<std::string, int> Query(int k) = 0;
    virtual void work() = 0;
    virtual void clear() = 0;
    virtual std::string get_name() = 0;
    virtual ~BaseSketch() {}
};

} // namespace sketch


#endif
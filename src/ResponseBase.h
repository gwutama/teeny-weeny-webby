#ifndef RESPONSEBASE_H
#define RESPONSEBASE_H

#include <string>

class ResponseBase {
public:
    virtual ~ResponseBase() = default;
    virtual std::string build_response() const = 0;
};


#endif //RESPONSEBASE_H

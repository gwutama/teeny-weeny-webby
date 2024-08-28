#ifndef RAWRESPONSE_H
#define RAWRESPONSE_H

#include "ResponseBase.h"

class RawResponse : public ResponseBase {
public:
    RawResponse();
    void set_raw_response(const std::string& raw_response_data);
    std::string build_response() const override;

private:
    std::string raw_response;
};


#endif //RAWRESPONSE_H

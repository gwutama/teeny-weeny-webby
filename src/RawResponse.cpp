#include "RawResponse.h"

RawResponse::RawResponse() {}

void RawResponse::set_raw_response(const std::string& raw_response_data) {
    raw_response = raw_response_data;
}

std::string RawResponse::build_response() const {
    return raw_response;
}

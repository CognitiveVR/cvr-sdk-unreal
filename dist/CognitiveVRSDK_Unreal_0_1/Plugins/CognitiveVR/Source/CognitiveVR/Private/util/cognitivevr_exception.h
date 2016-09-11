/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_COGNITIVEVR_RESPONSE_H_
#define COGNITIVEVR_COGNITIVEVR_RESPONSE_H_

#include <stdexcept>
#include "Private/network/cognitivevr_response.h"

//class CognitiveVRResponse;

//namespace cognitivevrapi
//{
    class cognitivevr_exception : public std::runtime_error
    {
        private:
			CognitiveVRResponse response;

        public:
            cognitivevr_exception(CognitiveVRResponse resp) : std::runtime_error(resp.GetErrorMessage().c_str()), response(resp){}
            ~cognitivevr_exception(void) throw(){}

			CognitiveVRResponse GetResponse()
            {
                return response;
            }
    };
//}
#endif  // COGNITIVEVR_COGNITIVEVR_RESPONSE_H_
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_NETWORK_DEFS_H_
#define COGNITIVEVR_NETWORK_DEFS_H_

//namespace cognitivevrapi
//{
    class CognitiveVRResponse
    {
        private:
            bool success;
            std::string error_message;
			FJsonObject content;

        public:
			CognitiveVRResponse(bool s) {
                success = s;
				//content = FJsonValue::EMPTY_OBJECT;
            }

            void SetErrorMessage(std::string err) {
                error_message = err;
            }

            void SetContent(FJsonObject c) {
                content = c;
            }

            bool IsSuccessful() {
                return success;
            }

            std::string GetErrorMessage() {
                return error_message;
            }

			FJsonObject GetContent() {
                return content;
            }
    };
	typedef void(*NetworkCallback)(CognitiveVRResponse);
//}
#endif  // COGNITIVEVR_NETWORK_DEFS_H_
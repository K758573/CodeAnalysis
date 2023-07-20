//
// Created by Shiro on 2023/7/20.
//

#ifndef CODEANALYSIS_CIPHERUTILS_H
#define CODEANALYSIS_CIPHERUTILS_H

#include <string>

class CipherUtils
{
public:
  static std::string Encrypt(std::string content);
  
  static std::string Decrypt(std::string data);
  
};


#endif //CODEANALYSIS_CIPHERUTILS_H

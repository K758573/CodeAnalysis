//
// Created by Shiro on 2023/7/20.
//

#include "CipherUtils.h"

std::string CipherUtils::Encrypt(std::string content)
{
  int Key[] = {1, 2, 3, 4, 5, 6, 7};
  for (uint32_t i = 0; i < content.length(); i++) {
    content[i] ^= Key[i % 7];
  }
  
  return content;
}

std::string CipherUtils::Decrypt(std::string data)
{
  int Key[] = {1, 2, 3, 4, 5, 6, 7};
  for (uint32_t i = 0; i < data.length(); i++) {
    data[i] ^= Key[i % 7];
  }
  
  return data;
}

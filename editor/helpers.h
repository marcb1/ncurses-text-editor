#pragma once

//openssl headers
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/err.h>
#include <openssl/sha.h>

//unix headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

//c++ headers
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <thread>
#include <mutex>


//constants
const int MAX_READ = 100;
const std::string SERVER_PASSWORD_FILE = "password_file";

inline bool writeSSL(const std::string& data, SSL* sslHandle)
{
  if(data.size() <= 0)
  {
    std::cerr << "Request to write 0 bytes, ignoring" << std::endl;
    return false;
  }
  unsigned int ret = SSL_write(sslHandle, data.c_str(), data.length());
  assert(ret == data.length());
  return (ret > 0);
}

inline std::string string_to_hex(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

inline std::string readSocket(SSL* sslHandle)
{
  // this is needed because the read is blocking, so we need to read everything in one chunk
  unsigned int maxRead = 1001;
  char readBuffer[maxRead];

  int readb = SSL_read(sslHandle, readBuffer, maxRead-1);
  if(readb <= 0)
  {
    std::cout << "Cannot read from socket; closing connection..." << std::endl;
    return std::string();
  }
  readBuffer[readb] = '\0';
  return std::string(readBuffer, readb);
}

//Open SSL wrapper functions
inline void loadOpenSSL()
{
  SSL_load_error_strings();
  SSL_library_init();
  OpenSSL_add_all_algorithms();
}

inline void unloadOpenSSL()
{
  ERR_free_strings();
  EVP_cleanup();
}

inline void deleteSSLStruct(SSL* ssl)
{
  SSL_shutdown(ssl);
  SSL_free(ssl);
}

//split string and update vector
inline std::vector<std::string>& splitString(const std::string &s, char delim, std::vector<std::string> &elems)
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim))
  {
    elems.push_back(item);
  }
  return elems;
}

//call delete when done
inline unsigned char* convertStringToChar(const std::string& s)
{
  unsigned char* ret = NULL;
  ret = new unsigned char[s.length()+1];
  strcpy((char *)ret, s.c_str());
  return ret;
}

inline std::string convertCharToString(unsigned char* c)
{
  std::string ret(reinterpret_cast<char*>(c));
  return ret;
}

inline void hextoChar(unsigned char* c, int size)
{
  for(int i = 0; i < size; i++)
  {
    std::cout << std::hex << (int)c[i];
  }
  std::cout << std::endl;
}

inline void printErrno()
{
  printf("Errno()! %s\n", strerror(errno));
}

inline off_t getFileSize(int fd)
{
  off_t file_len = lseek(fd, 0, SEEK_END);
  if(file_len == (off_t) -1)
  {
    return -1;
  }
  lseek(fd, 0, SEEK_SET);
  return file_len;
}

inline std::string simpleSHA256(const std::string& val)
{
  unsigned char* input = convertStringToChar(val);
  std::string ret;

  SHA256_CTX context;
  if(!SHA256_Init(&context))
    return ret;

  if(!SHA256_Update(&context, input, val.length()))
    return ret;

  unsigned char md[SHA256_DIGEST_LENGTH];
  if(!SHA256_Final(md, &context))
    return ret;

  ret = convertCharToString(md);
  delete input;

  return ret;
}

//Class that writes to log file and cleans up file descriptor correctly
class Logger
{
  private:
    std::ofstream logFile;

  public:
    Logger(const std::string& fileName):
      logFile(fileName.c_str(), std::ios_base::trunc)
  {
    std::cout << "opening log file: " << fileName << "; if this file already exists, it will overwritten..." << std::endl;
  }

    ~Logger()
    {
      logFile.close();
    }

    void append(const std::string& logText)
    {
      logFile << logText << std::flush;
    }
};

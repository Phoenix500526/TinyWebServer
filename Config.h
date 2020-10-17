#ifndef TINYWEBSERVER_CONFIG_H
#define TINYWEBSERVER_CONFIG_H

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class Key_Not_Found : public std::runtime_error {
 public:
  explicit Key_Not_Found(const std::string &str) : runtime_error(str) {}
};

class File_Not_Found : public std::runtime_error {
 public:
  explicit File_Not_Found(const std::string &str) : runtime_error(str) {}
};

class Config {
 private:
  std::string m_Delimiter;  //分隔符
  std::string m_Comment;    //注释符
  std::map<std::string, std::string> m_Content;
  using mapi = std::map<std::string, std::string>::iterator;
  using mapci = std::map<std::string, std::string>::const_iterator;

 public:
  Config(const char *file, std::string const delimiter,
         std::string const comment);
  Config(const char *file);
  std::string getDelimiter() { return m_Delimiter; }
  std::string getComment() { return m_Comment; }
  std::string setDelimiter(const std::string &new_delimiter) {
    std::string old_delimiter(m_Delimiter);
    m_Delimiter = new_delimiter;
    return old_delimiter;
  }
  std::string setComment(const std::string &new_comment) {
    std::string old_comment(m_Comment);
    m_Comment = new_comment;
    return old_comment;
  }
  template <typename T>
  void Read(std::string const &key, T &value);
  friend std::ostream &operator<<(std::ostream &os, const Config &cf);
  friend std::istream &operator>>(std::istream &is, Config &cf);

 private:
  static void Trim(std::string &str);
  template <typename T>
  T string_to_T(std::string const &value);
};

template <typename T>
void Config::Read(std::string const &key, T &value) {
  mapci p = m_Content.find(key);
  if (p == m_Content.end())
    throw Key_Not_Found(std::string("Cannot find the keyword : ") + key);
  value = string_to_T<T>(p->second);
}

template <typename T>
T Config::string_to_T(std::string const &value) {
  T t;
  std::istringstream ist(value);
  ist >> t;
  return t;
}

template <>
inline std::string Config::string_to_T<std::string>(std::string const &value) {
  return value;
}

template <>
inline bool Config::string_to_T<bool>(std::string const &value) {
  std::string tmp = value;
  for (auto iter = tmp.begin(); iter != tmp.end(); ++iter)
    *iter = toupper(*iter);
  return !(tmp == std::string("NO") || tmp == std::string("FALSE") ||
           tmp == std::string("0") || tmp == std::string("NONE") ||
           tmp == std::string("N") || tmp == std::string("F") ||
           tmp == std::string("CLOSE") || tmp == std::string("OFF"));
}
#endif

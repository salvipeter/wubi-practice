#ifndef WUBI_HH
#define WUBI_HH

#include <string>
#include <vector>

typedef std::vector<size_t> HanziString;
typedef std::vector<std::string> StringVector;

char rootCode(size_t index);
void putCharacter(int x, int y, size_t index, unsigned char color = 4);
void putString(int x, int y, std::string str, unsigned char color = 4);
void putHanziString(int x, int y, HanziString hs, unsigned char color = 4);
void putRomanString(int x, int y, std::string str, unsigned char color = 4);

#endif // WUBI_HH

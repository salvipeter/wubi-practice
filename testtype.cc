#include <algorithm>
#include <iostream>
#include <map>

#include "testtype.hh"

extern std::map<std::string, std::vector<HanziString> > wubi;
extern StringVector vkey[4], vmulti;

void TestType::display() const
{
  putRomanString(12, 62, "           ");
  putRomanString(12, 182, "           ");
  putRomanString(12, 302, "           ");
  for(PHVectorCIterator i = hanzi.begin(); i != hanzi.end(); ++i)
    putHanziString(i->x, i->y, i->hs, 4);
  putRomanString(12, 122, "           ");
  putRomanString(12, 242, "           ");
  putRomanString(12, 362, "           ");
}

void TestType::displayLastSolution()
{
  putString(236, 2, "上一个", 2);
  putRomanString(404, 2, last_sol, 2);
}

void TestType::displaySolution()
{
  putString(236, 2, "下一个", 2);
  putRomanString(404, 2, hanzi[next].sol, 2);
}

TestRoots::TestRoots() : TestType()
{
}

void TestRoots::generateTest()
{
  hanzi.clear();
  for(int i = 0; i < 33; ++i) {
    PositionedHanzi ph;
    ph.x = 12 + (i % 11) * 56;
    ph.y = 62 + (i / 11) * 120;
    ph.hs.push_back(7427 + (random() % 194));
    ph.sol = rootCode(ph.hs.back());
    hanzi.push_back(ph);
  }
  next = 0;
}

bool TestRoots::handleKey(char c, int &num, int &ok)
{
  if(c < 'a' || c >= 'z')
    return false;

  if(c == hanzi[next].sol[0]) {
    ++ok;
    putCharacter(hanzi[next].x, hanzi[next].y + 60, hanzi[next].hs[0], 2);
  } else
    putString(hanzi[next].x, hanzi[next].y + 60, "×", 1);

  last_sol = hanzi[next].sol;
  ++num; ++next;
  putRomanString(12, 2, "           ");
  return true;
}

TestNKey::TestNKey(int nkey) : TestType(), n(nkey), code("")
{
}

void TestNKey::generateTest()
{
  hanzi.clear();
  for(int i = 0; i < 33; ++i) {
    PositionedHanzi ph;
    ph.x = 12 + (i % 11) * 56;
    ph.y = 62 + (i / 11) * 120;
    ph.sol = vkey[n-1][random() % vkey[n-1].size()];
    std::vector<HanziString> tmp = wubi[ph.sol], tmp2;
    for(size_t i = 0; i < tmp.size(); ++i)
      if(tmp[i].size() == 1)
	tmp2.push_back(tmp[i]);
    ph.hs = tmp2[random() % tmp2.size()];
    hanzi.push_back(ph);
  }
  next = 0;
}

bool TestNKey::handleKey(char c, int &num, int &ok)
{
  if(c < 'a' || c >= 'z')
    return false;

  code += c;

  if(code.length() < hanzi[next].sol.length()) {
    putRomanString(12, 2, code, 1);
    return false;
  }

  std::vector<HanziString>::const_iterator i = wubi[code].begin();
  bool solved = false;
  while(i != wubi[code].end() && !solved)
    if((*i++)[0] == hanzi[next].hs[0])
      solved = true;

  if(solved) {
    ++ok;
    putCharacter(hanzi[next].x, hanzi[next].y + 60, hanzi[next].hs[0], 2);
  } else {
    bool one_hanzi = false;
    std::vector<HanziString>::const_iterator i;
    if(wubi.count(code) > 0) {
      for(i = wubi[code].begin(); i != wubi[code].end() && !one_hanzi; ++i)
	if(i->size() == 1)
	  one_hanzi = true;
    }
    if(one_hanzi)
      putCharacter(hanzi[next].x, hanzi[next].y + 60, (*(i-1))[0], 1);
    else
      putString(hanzi[next].x, hanzi[next].y + 60, "×", 1);
  }

  last_sol = hanzi[next].sol;
  ++num; ++next; code = "";
  putRomanString(12, 2, "           ");
  return true;
}

TestMulti::TestMulti() : TestType(), code("")
{
}

void TestMulti::generateTest()
{
  int last_x = -1, last_y = 1;
  hanzi.clear();
  while(true) {
    PositionedHanzi ph;
    ph.sol = vmulti[random() % vmulti.size()];
    std::vector<HanziString> tmp = wubi[ph.sol], tmp2;
    for(size_t i = 0; i < tmp.size(); ++i)
      if(tmp[i].size() > 1)
	tmp2.push_back(tmp[i]);
    ph.hs = tmp2[random() % tmp2.size()];
    ++last_x;
    if(last_x + ph.hs.size() > 10) {
      last_x = 0;
      last_y += 2;
    }
    if(last_y > 5)
      break;
    ph.x = 12 + last_x * 56;
    ph.y = 2 + last_y * 60;
    hanzi.push_back(ph);
    last_x += ph.hs.size();
  }
  next = 0;
}

bool TestMulti::handleKey(char c, int &num, int &ok)
{
  if(c < 'a' || c >= 'z')
    return false;

  code += c;

  if(code.length() < 4) {
    putRomanString(12, 2, code, 1);
    return false;
  }

  size_t nchar = hanzi[next].hs.size();

  std::vector<HanziString>::const_iterator i;
  bool solved = false;
  for(i = wubi[code].begin(); i != wubi[code].end() && !solved; ++i)
    if(i->size() == nchar) {
      int ok = true;
      for(size_t j = 0; j < i->size(); ++j)
	if((*i)[j] != hanzi[next].hs[j])
	  ok = false;
      if(ok)
	solved = true;
    }

  if(solved) {
    ok += nchar;
    putHanziString(hanzi[next].x, hanzi[next].y + 60, hanzi[next].hs, 2);
  } else {
    bool not_too_long = false;
    std::vector<HanziString>::const_iterator i;
    if(wubi.count(code) > 0) {
      for(i = wubi[code].begin(); i != wubi[code].end() && !not_too_long; ++i)
	if(i->size() <= nchar)
	  not_too_long = true;
    }
    if(not_too_long)
      putHanziString(hanzi[next].x, hanzi[next].y + 60, (*(i-1)), 1);
    else
      putString(hanzi[next].x, hanzi[next].y + 60, "×", 1);
  }

  last_sol = hanzi[next].sol;
  num += nchar; ++next; code = "";
  putRomanString(12, 2, "           ");
  return true;
}

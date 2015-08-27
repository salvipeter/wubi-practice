#ifndef TESTTYPE_HH
#define TESTTYPE_HH

#include <cstdlib>
#include <ctime>

#include "wubi.hh"

struct PositionedHanzi
{
  int x;
  int y;
  HanziString hs;
  std::string sol;
};
typedef std::vector<PositionedHanzi> PHVector;
typedef PHVector::const_iterator PHVectorCIterator;

class TestType
{
public:
  TestType() { next = 0; last_sol = "    "; srandom(time(NULL)); }
  virtual ~TestType() { }
  bool noNext() const { return (next >= hanzi.size()); }
  virtual void generateTest() = 0;
  void display() const;
  void displayLastSolution();
  void displaySolution();
  virtual bool handleKey(char c, int &num, int &ok) = 0;
protected:
  size_t next;
  std::string last_sol;
  PHVector hanzi;
};

class TestRoots : public TestType
{
public:
  TestRoots();
  void generateTest();
  bool handleKey(char c, int &num, int &ok);
};

class TestNKey : public TestType
{
public:
  TestNKey(int nkey);
  void generateTest();
  bool handleKey(char c, int &num, int &ok);
private:
  int n;
  std::string code;
};

class TestMulti: public TestType
{
public:
  TestMulti();
  void generateTest();
  bool handleKey(char c, int &num, int &ok);
private:
  std::string code;
};

#endif // TESTTYPE_HH

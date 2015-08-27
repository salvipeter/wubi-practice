// Wubi input method trainer
//
// Copyright (C) 2007 Peter Salvi <vukung@yahoo.com>
// 
// Hanzi bitmap font is taken from the tetex package (jfs56),
// the wubi database is based on a file from the cjkvinput project,
// the origin of the two JPG files is unknown;
// the pictures of the roots were extracted from the file wubi-keys.jpg.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <SDL.h>

#include "testtype.hh"
#include "wubi.hh"

int const width = 640, height = 480;

std::vector<bool *> font;
std::map<std::string, std::vector<HanziString> > wubi;
StringVector vkey[4], vmulti;
SDL_Surface *screen;

bool readFontFile(const std::string &fname)
{
  char buf[392];		// 56*56 font, 1 character stored in 392 bytes
  std::ifstream f(fname, std::ios::in | std::ios::binary);
  if (!f.is_open())
    return false;
  while(!f.eof()) {
    f.read(buf, 392);
    bool *hanzi = new bool[56 * 56];
    for(int j = 0; j < 56; ++j)
      for(int i = 0; i < 56; ++i) {
	hanzi[j * 56 + i] = buf[j * 7 + i / 8] & (1 << (55 - i) % 8);
      }
    font.push_back(hanzi);
  }
  f.close();
  return true;
}

size_t getIndex(size_t code)	// converts the gb2312 code to index number
{
  size_t const first = code >> 8;
  size_t const second = code - (first << 8);

  size_t offset, start;
  if(first < 0xB0) {
    offset = 0;
    start = 0xA1;
  }
  else {
    offset = 658;
    start = 0xB0;
  }

  // The 0xA1-0xFE range has 94 elements
  return offset + 94 * (first - start) + (second - 0xA1);
}

HanziString convertToIndex(const std::string &str)
{
  size_t i = 0;
  HanziString hs;
  while(i < str.length()) {
    size_t const first  = (unsigned char)str[i++];
    size_t const second = (unsigned char)str[i++];
    hs.push_back(getIndex(first * 256 + second));
  }
  return hs;
}

bool readWubiFile(const std::string &fname)
{
  std::string s;
  std::ifstream f(fname);
  if (!f.is_open())
    return false;
  std::map<std::string, std::string> reverse;
  while(!f.eof()) {
    std::getline(f, s);
    size_t const tabpos = s.find('\t');
    std::string const code = s.substr(0, tabpos);
    std::string const hanzi = s.substr(tabpos + 1);
    wubi[code].push_back(convertToIndex(hanzi));
    if(reverse.count(hanzi) > 0) {
      if(reverse[hanzi].length() > code.length())
	reverse[hanzi] = code;
    } else
      reverse[hanzi] = code;
  }
  f.close();
  for(std::map<std::string, std::string>::const_iterator i = reverse.begin();
      i != reverse.end(); ++i) {
    switch(i->second.length()) {
    case 1: vkey[0].push_back(i->second); break;
    case 2: vkey[1].push_back(i->second); break;
    case 3: vkey[2].push_back(i->second); break;
    default:
      if(i->first.length() > 2)
	vmulti.push_back(i->second);
      else
	vkey[3].push_back(i->second);
    }
  }
  return true;
}

char rootCode(size_t index)
{
  char const codes[] = { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
			 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
			 'x', 'c', 'v', 'b', 'n', 'm' };
  size_t const entries[] = { 12, 5, 13, 8, 8, 9, 9, 8, 4, 6,
			     8, 3, 12, 8, 5, 9, 9, 3, 10,
			     6, 6, 6, 10, 11, 6 };
  index -= 7427;

  size_t i = 0, offs = 0;
  do {
    offs += entries[i++];
  } while(offs <= index);

  return codes[i-1];
}

void setPalette()
{
  SDL_Color colors[256];
  colors[0].r = 0; colors[0].g = 0; colors[0].b = 0;
  colors[1].r = 255; colors[1].g = 0; colors[1].b = 0;
  colors[2].r = 0; colors[2].g = 255; colors[2].b = 0;
  colors[3].r = 0; colors[3].g = 0; colors[3].b = 255;
  colors[4].r = 120; colors[4].g = 120; colors[4].b = 120;
  for(int i = 5; i < 256; ++i) {
    colors[i].r = 0; colors[i].g = 0; colors[i].b = 0;
  }
  SDL_SetPalette(screen, SDL_PHYSPAL, colors, 0, 256);
}

void putCharacter(int x, int y, size_t index, unsigned char color)
{
  SDL_LockSurface(screen);
  Uint8 *memory = (Uint8 *)screen->pixels + y * screen->pitch + x;
  for(int j = 0; j < 56; ++j)
    for(int i = 0; i < 56; ++i)
      memory[j * screen->pitch + i] = font[index][j * 56 + i] ? color : 0;
  SDL_UnlockSurface(screen);
  SDL_UpdateRect(screen, x, y, 56, 56);
}

void putString(int x, int y, std::string str, unsigned char color)
{
  HanziString hs = convertToIndex(str);
  for(size_t i = 0; i < hs.size(); ++i)
    putCharacter(x + i * 56, y, hs[i], color);
}

void putHanziString(int x, int y, HanziString hs, unsigned char color)
{
  for(size_t i = 0; i < hs.size(); ++i)
    putCharacter(x + i * 56, y, hs[i], color);
}

void putRomanString(int x, int y, std::string str, unsigned char color)
{
  for(size_t i = 0; i < str.length(); ++i)
    putCharacter(x + i * 56, y, 155 + (unsigned int)str[i], color);
}

void putNumber(int x, int y, unsigned int num)
{
  int const ones = num % 10;
  int const tens = (num % 100) / 10;
  int const hundreds = (num % 1000) / 100;
  putCharacter(x, y, 203 + hundreds, 3);
  putCharacter(x + 56, y, 203 + tens, 3);
  putCharacter(x + 112, y, 203 + ones, 3);
}

void clearScreen()
{
  SDL_LockSurface(screen);
  Uint8 *memory = (Uint8 *)screen->pixels;
  for(int j = 0; j < height; ++j)
    for(int i = 0; i < width; ++i)
      memory[j * screen->pitch + i] = 0;
  SDL_UnlockSurface(screen);
  SDL_UpdateRect(screen, 0, 0, width, height);
}

void drawMenu()
{
  putString(12, 2, "五笔练习", 1);
  putString(12, 62, "㈠部首");
  putString(12, 122, "㈡一字一键");
  putString(12, 182, "㈢一字二键");
  putString(12, 242, "㈣一字三键");
  putString(12, 302, "㈤一字四键");
  putString(12, 362, "㈥多字");
  putString(12, 422, "㈦退出");
}

void practice(TestType *prac)
{
  clearScreen();
  putString(12, 422, "速度", 3);
  putString(292, 422, "准确度", 3);

  prac->generateTest();
  prac->display();

  SDL_Event event;
  bool quit = false;
  int num = 0, ok = 0;
  Uint32 start = SDL_GetTicks();
  while(!quit) {
    if(prac->noNext()) {
      Uint32 const start_waiting = SDL_GetTicks();
      putRomanString(12, 2, "           ");
      putString(124, 2, "请按任意键继续", 3);
      do {
	SDL_WaitEvent(&event);
      } while(event.type != SDL_KEYUP);

      putRomanString(12, 2, "           ");
      prac->generateTest();
      prac->display();
      start += SDL_GetTicks() - start_waiting;
    }
    SDL_WaitEvent(&event);
    if(event.type == SDL_KEYUP) {
      switch(event.key.keysym.sym) {
      case SDLK_ESCAPE: quit = true; break;
      case SDLK_BACKSLASH: prac->displayLastSolution(); break;
      case SDLK_z: prac->displaySolution(); break;
      default:
	if(prac->handleKey(event.key.keysym.sym, num, ok)) {
	  Uint32 const elapsed = SDL_GetTicks() - start;
	  unsigned int const speed = 60000 * num / elapsed;
	  unsigned int const accuracy = ok * 100 / num;
	  putNumber(124, 422, speed);
	  putNumber(460, 422, accuracy);
	}
      }
    }
  }
}

int main(int argc, char *argv[])
{
  std::cout << "Reading font files... " << std::flush;
  font.reserve(8000);
  if (!readFontFile("font.dat")) {	// 7427 characters
    std::cerr << "error reading 'font.dat'" << std::endl;
    return 1;
  }
  if (!readFontFile("roots.dat")) {	//  194 characters
    std::cerr << "error reading 'roots.dat'" << std::endl;
    return 1;
  }
  std::cout << "done" << std::endl;

  std::cout << "Reading wubi database... " << std::flush;
  if (!readWubiFile("wubi.dat")) {
    std::cerr << "error reading 'wubi.dat'" << std::endl;
    return 1;
  }
  std::cout << "done" << std::endl;

  if(SDL_Init(SDL_INIT_VIDEO) == -1) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
    return 1;
  }

  if((screen = SDL_SetVideoMode(width, height, 8,
				SDL_HWSURFACE | SDL_HWPALETTE)) == NULL) {
    std::cerr << "Unable to set video mode: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_WM_SetCaption("Wubi Practice", "");
  setPalette();

  drawMenu();

  SDL_Event event;
  bool quit = false, action = false;
  TestType *prac;
  while(!quit) {
    SDL_WaitEvent(&event);
    if(event.type == SDL_KEYDOWN) {
      switch(event.key.keysym.sym) {
      case SDLK_1: prac = new TestRoots(); action = true; break;
      case SDLK_2: prac = new TestNKey(1); action = true; break;
      case SDLK_3: prac = new TestNKey(2); action = true; break;
      case SDLK_4: prac = new TestNKey(3); action = true; break;
      case SDLK_5: prac = new TestNKey(4); action = true; break;
      case SDLK_6: prac = new TestMulti(); action = true; break;
      case SDLK_7:
      case SDLK_q: quit = true; break;
      default: ;
      }
    }
    if(action) {
      practice(prac);
      delete prac;
      clearScreen();
      drawMenu();
      action = false;
    }
  }

  // Free the font memory
  for(std::vector<bool *>::iterator i = font.begin(); i != font.end(); ++i)
    delete[] *i;

  SDL_Quit();

  return 0;
}

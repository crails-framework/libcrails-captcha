#include "default.hpp"
#include <crails/read_file.hpp>
#include <crails/logger.hpp>
#include <libcapt/captGenerator.h>
#include <libcapt/captFontFile.h>
#include <libcapt/captRLE.h>
#include <Magick++.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Crails;

#pragma pack(push, 1)
struct TgaHead
{
  unsigned char  ident_size = 0;
  unsigned char  color_map_type = 0;
  unsigned char  image_type = 0;
  unsigned short color_map_start = 0;
  unsigned short color_map_length = 0;
  unsigned char  color_map_bits = 0;
  unsigned short x_start, y_start = 0;
  unsigned short width, height = 0;
  unsigned char  bits = 0;
  unsigned char  descriptor = 0;
};
#pragma pack(pop)

static const libCapt::FontFile& get_font_file(const std::string& font_filename = "")
{
  static libCapt::FontFile font;
  static bool initialized = false;

  if (!initialized)
  {
    std::string file_contents;

    if (Crails::read_file(font_filename, file_contents))
    {
      initialized = true;
      font.loadFromDataStream(reinterpret_cast<const unsigned char*>(file_contents.c_str()), file_contents.length());
    }
    else
      logger << Logger::Error << "libcrails-captcha: could not open font file " << font_filename << Logger::endl;
  }
  return font;
}

static TgaHead make_tga_header()
{
  TgaHead tga_header;

  tga_header.image_type = 2;
  tga_header.width = libCapt::Question::IMAGE_WIDTH;
  tga_header.height = libCapt::Question::IMAGE_HEIGHT;
  tga_header.bits = 24;
  return tga_header;
}

static const TgaHead tga_header = make_tga_header();

static string captcha_to_tga(const libCapt::Question& question)
{
  stringstream stream;
  unsigned char buffer[libCapt::Question::IMAGE_BUF_LENGTH];

  if (question.isCompressed())
  {
    unsigned int rle_size = question.getSize();
    unsigned int buffer_size = libCapt::Question::IMAGE_BUF_LENGTH;

    rleDecompress(
      question.imageBuf, rle_size,
      libCapt::Question::IMAGE_WIDTH, libCapt::Question::IMAGE_HEIGHT,
      (unsigned char*)buffer, buffer_size
    );
  }
  else 
  {
    memcpy(buffer, question.imageBuf, libCapt::Question::IMAGE_BUF_LENGTH);
  }
  stream.write(reinterpret_cast<const char*>(&tga_header), sizeof(tga_header));
  for (int y = 0 ; y < libCapt::Question::IMAGE_HEIGHT ; y++)
  {
    for (int x = 0 ; x < libCapt::Question::IMAGE_WIDTH ; x++)
    {
      unsigned char c = buffer[(libCapt::Question::IMAGE_HEIGHT-1-y)*libCapt::Question::IMAGE_PITCH + (x>>1)];
      c = (x&1) ? ((c&0xf)<<4) : (c&0xf0);
      unsigned char pixel[4]={c,c,c,c};                                                             
      stream.write(reinterpret_cast<const char*>(pixel), 3);
    }                        
  }
  return stream.str();
}

static string base64_captcha(const libCapt::Question& question)
{
  string        tga = captcha_to_tga(question);
  Magick::Blob  blob(reinterpret_cast<const void*>(tga.c_str()), tga.length());
  Magick::Image image;
  Magick::Blob  jpg;

  image.magick("TGA");
  image.read(blob);
  image.rotate(180);
  image.flop();
  image.magick("JPG");
  image.write(&jpg);
  return jpg.base64(); 
}

static string get_challenge_solution(const libCapt::Question& challenge)
{
  string solution;
  const unsigned short* widechars_solution = challenge.answer();

  solution.reserve(libCapt::Question::ANSWER_LENGTH);
  for (unsigned short i = 0 ; i < libCapt::Question::ANSWER_LENGTH ; ++i)
    solution += static_cast<char>(widechars_solution[i]);
  cout << "SOLUTION: " << solution << endl;
  return solution;
}

string Captcha::Default::render(Params& params) const
{
  const libCapt::FontFile& font = get_font_file(); 
  Data session = params.get_session();
  libCapt::Question challenge;
  libCapt::Generator generator(font);
  stringstream html;

  generator.generateQuestion(challenge);
  session["captcha"] = get_challenge_solution(challenge);
  html
    << "<div class=\"crails-captcha\">"
    <<   "<img src=\"data:image/tga;charset=utf-8;base64," << base64_captcha(challenge) << "\"/>"
    <<   "<input id=\"" << get_element_id() << "\" type=\"text\" name=\"" << get_form_key() << "\" />"
    << "</div>";
  return html.str();
}

void Captcha::Default::check(Params& params, function<void(bool)> callback) const
{
  Data candidate = params[get_form_key().data()];
  Data solution = params.get_session()["captcha"];

  callback(
       candidate.exists()
    && solution.exists()
    && candidate.as<string>() == solution.as<string>()
  );
}

void Captcha::Default::set_font_file(const std::string& filepath)
{
  get_font_file(filepath);
}

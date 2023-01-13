#ifndef  CRAILS_CAPTCHA_DEFAULT_HPP
# define CRAILS_CAPTCHA_DEFAULT_HPP

# include "../captcha.hpp"

namespace Crails
{
  namespace Captcha
  {
    class Default : public Challenge
    {
    public:
      std::string render(Crails::Params&) const override;
      void check(Crails::Params&, std::function<void(bool)> callback) const override;

      static void set_font_file(const std::string&);
    };
  }
}

#endif

#ifndef  CRAILS_CAPTCHA_HPP
# define CRAILS_CAPTCHA_HPP

# include <crails/params.hpp>
# include <string_view>

namespace Crails
{
  namespace Captcha
  {
    class Challenge
    {
    public:
      virtual std::string_view get_form_key() const { return "crails-captcha"; }
      virtual std::string_view get_element_id() const { return get_form_key(); }

      virtual std::string render(Crails::Params&) const = 0;
      virtual void check(Crails::Params&, std::function<void(bool)> callback) const = 0;
    };
  }
}

#endif

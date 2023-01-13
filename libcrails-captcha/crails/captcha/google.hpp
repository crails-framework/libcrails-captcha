#ifndef  CRAILS_CAPTCHA_GOOGLE_HPP
# define CRAILS_CAPTCHA_GOOGLE_HPP

# include "../captcha.hpp"

namespace Crails
{
  namespace Captcha
  {
    class Google : public Challenge
    {
    public:
      virtual std::string_view get_api_key() const = 0;
      virtual std::string_view get_api_secret() const = 0;
      virtual double get_score_threshold() const { return 0.5; }

      std::string render(Crails::Params&) const override;
      void check(Crails::Params&, std::function<void(bool)> callback) const override;
    private:
      std::string get_verify_url(const std::string& captcha_id) const;
    };
  }
}

#endif

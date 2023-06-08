#include "google.hpp"
#include <sstream>
#include <crails/client.hpp>
#include <crails/logger.hpp>

using namespace std;
using namespace Crails;

string Captcha::Google::render(Params&) const
{
  stringstream html;

  html
    << "<script src=\"https://www.google.com/recaptcha/api.js?render=" << get_api_key() << "\"></script>"
    << "<input type=\"hidden\" id=\"" << get_element_id() <<"\" name=\"" << get_form_key() << "\">"
    << "<input type=\"hidden\" name=\"action\" value=\"validate_captcha\">";
  html
    << "<script>"
    <<   "grecaptcha.ready(function() {"
    <<     "grecaptcha.execute('" << get_api_key() << "',{action:'validate_captcha'})"
    <<       ".then(function(token) {"
    <<         "document.getElementById('" << get_element_id() << "').value = token;"
    <<       '}'
    <<   '}'
    << "</script>";
  return html.str();
}

void Captcha::Google::check(Params& params, function<void(bool)> callback) const
{
  Data captcha_id = params[get_form_key().data()];

  if (captcha_id.exists())
  {
    auto http = make_shared<Ssl::Client>("www.google.com");
    Client::Request request{HttpVerb::get, get_verify_url(captcha_id), 11};

    logger << Logger::Debug << "Performing Captcha::Google::check" << Logger::endl;
    http->async_query(request, [this, http, callback](const Client::Response& response, boost::beast::error_code)
    {
      DataTree body;

      body.from_json(response.body());
      Data status = body["success"];
      logger
        << Logger::Debug
        << function<string()>([&body]() { return "Received Captcha::Google::check response:\n" + body.to_json(); })
        << Logger::endl;
      if (status.exists() && status.as<string>() != "false")
        callback(body["score"].as<double>() > get_score_threshold());
      else
        callback(false);
    });
  }
  else
    callback(false);
}

string Captcha::Google::get_verify_url(const string& captcha_id) const
{
  stringstream url;

  url << "/recaptcha/api/siteverify"
      << "?secret=" << get_api_secret()
      << "&response=" << captcha_id;
  return url.str();
}

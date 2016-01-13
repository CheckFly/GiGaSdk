#include "HttpClient.h"
#include <cpprest/http_client.h>
#include <boost/regex.hpp>

#include "Empty.h"
#include "JsonObj.h"
#include "../Config.h"

using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;

namespace giga
{

HttpClient::HttpClient () :
        client (HttpClient::HOST, getConfig ())
{
}

web::uri_builder
HttpClient::uri (const std::string& resource)
{
    return web::uri_builder
        {API + resource};
}

struct Redirect {
    std::string redirect = "";
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, redirect);
    }
};

void
HttpClient::authenticate (const std::string& login, const std::string& password)
{
    const auto& conf = Config::getInstance();
    oauth2_config m_oauth2_config(conf.getAppId(), conf.getAppKey(), conf.getAppOauthAuthorizationEndpoint(),
                                  conf.getAppOauthTokenEndpoint(), conf.getAppRedirectUri());

    m_oauth2_config.set_scope(conf.getAppScope());
    auto auth_uri = m_oauth2_config.build_authorization_uri(true);  /* Get the authorization uri */
    auto state  = std::string{};
    auto regex  = boost::regex{".*state=([a-zA-Z0-9]+).*"};
    auto what   = boost::cmatch{};
    if(boost::regex_match(auth_uri.c_str(), what, regex))
    {
        state = std::string{what[1].first, what[1].second};
    }

    // I manually do the browser work here ...

    auto body = JsonObj{};
    body.add("login", login);
    body.add("password", password);
    auto url = "/rest/login";
    auto request = client.request(methods::POST, url, JSonSerializer::toString(body), JSON_CONTENT_TYPE).then([=](web::http::http_response response) {
        onRequest<Empty>(response);
        auto headers = response.headers();

        const auto& conf = Config::getInstance();
        auto body = JsonObj{};
        body.add("oauth", std::string("true"));
        body.add("response_type", std::string("code"));
        body.add("client_id", conf.getAppId());
        body.add("redirect_uri", conf.getAppRedirectUri());
        body.add("state", state);
        body.add("scope", conf.getAppScope());
        body.add("authorized", true);

        auto r = http_request{methods::POST};
        r.set_request_uri("/rest/oauthvalidate");
        r.set_body(JSonSerializer::toString(body), JSON_CONTENT_TYPE);
        auto it = headers.find("Set-Cookie");
        if (it != headers.end()) {
            r.headers().add("Cookie", it->second);
        }
        auto request = client.request(r).then([=](web::http::http_response response) -> std::string {
            auto redirect = onRequest<Redirect>(response);
            return redirect.redirect;
        });
        request.wait();
        return request.get();
    });
        request.wait();

        auto uri = uri_builder{request.get()}.to_uri();
        m_oauth2_config.token_from_redirected_uri(uri).wait();

        auto config = getConfig();
        config.set_oauth2(m_oauth2_config);
        client = {HttpClient::HOST, config};
}


}

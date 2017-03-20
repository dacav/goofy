#include "node-redirect.h"

#include <regex>

namespace spg::gopher
{

    NodeRedirect::NodeRedirect(const settings::Settings& sets) :
        Node(
            NodeType::NT_HYPERTEXT,
            "URL resolver",
            "URL:",
            sets.host_name,
            sets.listen_port
        ),
        settings(sets)
    {
    }

    std::unique_ptr<proto::Writer> NodeRedirect::make_writer(
            const WriteParams& wp,
            const request::Request& req) const
    {
        proto::BytesWriter* writer = new proto::BytesWriter(wp);
        std::unique_ptr<Writer> out(writer);

        // TODO: have these in a conf.h with autotools
#define SERVER "spg"
#define VERSION "0.0"

        std::string text = (
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n"
            "<html>\n"
            "<head>\n"
            "<meta http-equiv=\"refresh\" content=\"5;URL=${URL}/\">\n"
            "<title>Redirection to URL: ${URL}\n"
            "</head>\n"
            "<body>\n"
            "<p>\n"
            "You are following a link from gopher to another URL or protocol. You\n"
            "will be automatically taken to the site shortly.  If you don't get\n"
            "sent there, please click <a href=\"${URL}\">here</a> to go to the site.\n"
            "</p>\n"
            "<p>\n"
            "The URL linked is:\n"
            "</p>\n"
            "<p>\n"
            "<a href=\"${URL}\">${URL}</a>\n"
            "</p>\n"
            "<hr>\n"
            "<address>generated by " SERVER " " VERSION "</address>\n"
            "</body>\n"
            "</html>\n"
        );

        constexpr const char* keyword = "${URL}";
        constexpr size_t len = strlen(keyword);

        auto next = text.find("${URL}");
        while (next != text.npos) {
            text.replace(next, len, req.url());
            next = text.find("${URL}");
        }
        writer->append(text);
        return out;
    }


}

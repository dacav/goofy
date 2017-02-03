#include "node.h"
#include "proto.h"

namespace spg::gopher
{

    Node::Node(NodeType _type,
               const std::string& _display_name,
               const std::string& _selector,
               const std::string& _host,
               uint16_t _port)
        : type(_type)
        , display_name(_display_name)
        , selector(_selector)
        , host(_host)
        , port(_port)
    {
        for (auto &s : {display_name, selector, host}) {
            if (s.find('\t') != std::string::npos) {
                throw NodeError(std::string("Unallowed tab sign: ") + s);
            }
        }
    }

    void Node::repr(int fd) const
    {
        using spg::gopher::proto::write;
        char t = type;
        write(fd, &t, sizeof(t));

        using spg::gopher::proto::writetb;
        writetb(fd, display_name);
        writetb(fd, selector);
        writetb(fd, host);

        using spg::gopher::proto::writeln;
        writeln(fd, std::to_string(port));
    }
}

#pragma once

namespace spg::gopher
{
    enum NodeType {
        NT_PLAIN = '0',
        NT_DIRLIST = '1',
        NT_ERROR = '3',
        NT_INFO = 'i',
    };
}

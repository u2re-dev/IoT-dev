#pragma once

#include "./tlv.h"
#include "./std/tree.h"
#include <stdexcept>

namespace tlvcpp
{
    using tlv_tree_node = tree_node<tlv>;
}

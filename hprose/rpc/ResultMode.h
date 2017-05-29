/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * ResultMode.h                                           *
 *                                                        *
 * hprose ResultMode for cpp.                             *
 *                                                        *
 * LastModified: May 02, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

namespace hprose {
namespace rpc {

enum ResultMode {
    Normal,
    Serialized,
    Raw,
    RawWithEndTag
};

}
} // namespace hprose::rpc
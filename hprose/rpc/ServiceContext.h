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
 * hprose/rpc/ServiceContext.h                            *
 *                                                        *
 * hprose service context for cpp.                        *
 *                                                        *
 * LastModified: May 02, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

#include <hprose/rpc/Context.h>
#include <hprose/io/Reader.h>
#include <hprose/io/Writer.h>

#include <sstream>

namespace hprose {
namespace rpc {

class ServiceContext : public Context {
public:
    std::stringstream rstream;
    std::ostringstream wstream;

    hprose::io::Reader reader;
    hprose::io::Writer writer;
    bool oneway;
    bool byref;

    ServiceContext() : reader(rstream, false),
        writer(wstream, true),
        oneway(false),
        byref(false) {
    }
};

}
} // namespace hprose::rpc
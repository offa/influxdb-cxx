///
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_TRANSPORTS_HTTP_H
#define INFLUXDATA_TRANSPORTS_HTTP_H

#include "Transport.h"
#include <curl/curl.h>
#include <memory>
#include <string>

namespace influxdb
{
namespace transports
{

/// \brief HTTP transport
class HTTP : public Transport
{
  public:
    /// Constructor
    HTTP(const std::string& url);

    /// Default destructor
    ~HTTP() = default;

    /// Sends point via HTTP POST
    void send(std::string&& post);
    
  private:
    /// Custom deleter of CURL object
    static void deleteCurl(CURL * curl);

    /// Initilizes CURL and all common options
    CURL* initCurl(const std::string& url);

    /// CURL smart pointer with custom deleter
    std::unique_ptr<CURL, decltype(&HTTP::deleteCurl)> curlHandle;
};

} // namespace transports
} // namespace influxdb

#endif // INFLUXDATA_TRANSPORTS_HTTP_H

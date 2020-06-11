///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "HTTP.h"
#include "InfluxDBException.h"
#include <iostream>

namespace influxdb::transports
{

HTTP::HTTP(const std::string &url)
{
  initCurl(url);
  initCurlRead(url);
  obtainInfluxServiceUrl(url);
  obtainDatabaseName(url);
}

void HTTP::initCurl(const std::string &url)
{
  CURLcode globalInitResult = curl_global_init(CURL_GLOBAL_ALL);
  if (globalInitResult != CURLE_OK)
  {
    throw InfluxDBException(__PRETTY_FUNCTION__, curl_easy_strerror(globalInitResult));
  }

  std::string writeUrl = url;
  auto position = writeUrl.find("?");
  if (position == std::string::npos)
  {
    throw InfluxDBException(__PRETTY_FUNCTION__, "Database not specified");
  }
  if (writeUrl.at(position - 1) != '/')
  {
    writeUrl.insert(position, "/write");
  }
  else
  {
    writeUrl.insert(position, "write");
  }
  writeHandle = curl_easy_init();
  curl_easy_setopt(writeHandle, CURLOPT_URL, writeUrl.c_str());
  curl_easy_setopt(writeHandle, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(writeHandle, CURLOPT_CONNECTTIMEOUT, 10);
  curl_easy_setopt(writeHandle, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(writeHandle, CURLOPT_POST, 1);
  curl_easy_setopt(writeHandle, CURLOPT_TCP_KEEPIDLE, 120L);
  curl_easy_setopt(writeHandle, CURLOPT_TCP_KEEPINTVL, 60L);
  FILE *devnull = fopen("/dev/null", "w+");
  curl_easy_setopt(writeHandle, CURLOPT_WRITEDATA, devnull);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *) userp)->append((char *) contents, size * nmemb);
  return size * nmemb;
}

void HTTP::initCurlRead(const std::string &url)
{
  mReadUrl = url + "&q=";
  mReadUrl.insert(mReadUrl.find("?"), "/query");
  readHandle = curl_easy_init();
  curl_easy_setopt(readHandle, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(readHandle, CURLOPT_CONNECTTIMEOUT, 10);
  curl_easy_setopt(readHandle, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(readHandle, CURLOPT_TCP_KEEPIDLE, 120L);
  curl_easy_setopt(readHandle, CURLOPT_TCP_KEEPINTVL, 60L);
  curl_easy_setopt(readHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
}

std::string HTTP::query(const std::string &query)
{
  CURLcode response;
  long responseCode;
  std::string buffer;
  char *encodedQuery = curl_easy_escape(readHandle, query.c_str(), query.size());
  auto fullUrl = mReadUrl + std::string(encodedQuery);
  curl_easy_setopt(readHandle, CURLOPT_URL, fullUrl.c_str());
  curl_easy_setopt(readHandle, CURLOPT_WRITEDATA, &buffer);
  response = curl_easy_perform(readHandle);
  curl_easy_getinfo(readHandle, CURLINFO_RESPONSE_CODE, &responseCode);
  treatCurlResponse(response, responseCode);
  return buffer;
}

void HTTP::enableBasicAuth(const std::string &auth)
{
  curl_easy_setopt(writeHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(writeHandle, CURLOPT_USERPWD, auth.c_str());
  curl_easy_setopt(readHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(readHandle, CURLOPT_USERPWD, auth.c_str());
}

void HTTP::enableSsl()
{
  curl_easy_setopt(readHandle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(writeHandle, CURLOPT_SSL_VERIFYPEER, 0L);
}

HTTP::~HTTP()
{
  curl_easy_cleanup(writeHandle);
  curl_easy_cleanup(readHandle);
  curl_global_cleanup();
}

void HTTP::send(std::string &&lineprotocol)
{
  CURLcode response;
  long responseCode;
  curl_easy_setopt(writeHandle, CURLOPT_POSTFIELDS, lineprotocol.c_str());
  curl_easy_setopt(writeHandle, CURLOPT_POSTFIELDSIZE, (long) lineprotocol.length());
  response = curl_easy_perform(writeHandle);
  curl_easy_getinfo(writeHandle, CURLINFO_RESPONSE_CODE, &responseCode);
  treatCurlResponse(response, responseCode);
}

void HTTP::treatCurlResponse(const CURLcode &response, long responseCode) const
{
  if (response != CURLE_OK)
  {
    throw ConnectionError(__PRETTY_FUNCTION__, curl_easy_strerror(response));
  }
  //
  // Influx API response codes:
  // https://docs.influxdata.com/influxdb/v1.7/tools/api/#status-codes-and-responses-2
  //
  if (responseCode == 404)
  {
    throw NonExistentDatabase(__PRETTY_FUNCTION__, "Nonexistent database: " + std::to_string(responseCode));
  }
  else if ((responseCode >= 400) && (responseCode < 500))
  {
    throw BadRequest(__PRETTY_FUNCTION__, "Bad request: " + std::to_string(responseCode));
  }
  else if (responseCode > 500)
  {
    throw ServerError(__PRETTY_FUNCTION__, "Influx server error:" + std::to_string(responseCode));
  }
}

void HTTP::obtainInfluxServiceUrl(const std::string &url)
{
  auto questionMarkPosition = url.find("?");
  mInfluxDbServiceUrl = url.substr(0, questionMarkPosition);
}

void HTTP::obtainDatabaseName(const std::string &url)
{
  auto dbParameterPosition = url.find("db=");
  mDatabaseName = url.substr(dbParameterPosition + 3);
}

std::string HTTP::databaseName() const
{
  return mDatabaseName;
}

std::string HTTP::influxDbServiceUrl() const
{
  return mInfluxDbServiceUrl;
}

void HTTP::createDatabase()
{
  std::string createUrl = mInfluxDbServiceUrl + "/query";
  std::string postFields = "q=CREATE DATABASE " + mDatabaseName;

  CURL *createHandle = curl_easy_init();
  curl_easy_setopt(createHandle, CURLOPT_URL, createUrl.c_str());
  curl_easy_setopt(createHandle, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(createHandle, CURLOPT_CONNECTTIMEOUT, 10);
  curl_easy_setopt(createHandle, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(createHandle, CURLOPT_POST, 1);
  curl_easy_setopt(createHandle, CURLOPT_TCP_KEEPIDLE, 120L);
  curl_easy_setopt(createHandle, CURLOPT_TCP_KEEPINTVL, 60L);
  FILE *devnull = fopen("/dev/null", "w+");
  curl_easy_setopt(createHandle, CURLOPT_WRITEDATA, devnull);

  curl_easy_setopt(createHandle, CURLOPT_POSTFIELDS, postFields.c_str());
  curl_easy_setopt(createHandle, CURLOPT_POSTFIELDSIZE, (long) postFields.length());

  CURLcode response = curl_easy_perform(createHandle);
  long responseCode;
  curl_easy_getinfo(createHandle, CURLINFO_RESPONSE_CODE, &responseCode);
  treatCurlResponse(response,responseCode);
}

} // namespace influxdb

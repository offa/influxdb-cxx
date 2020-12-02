// MIT License
//
// Copyright (c) 2019 Adam Wegrzynek
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define BOOST_TEST_MODULE Test InfluxDB
#define BOOST_TEST_DYN_LINK

#include <boost/test/included/unit_test.hpp>
#include <random>

#include "HTTP.h"
#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

namespace influxdb::test
{
    namespace
    {
        /**
         * \deprecated related test has been moved to unit test
         */
        class WriteRecorder
        {
        public:
            void record([[maybe_unused]] std::string message)
            {
                ++calls;
            }

            std::size_t count() const
            {
                return calls;
            }

        private:
            std::size_t calls{0};
        };

        /**
         * \deprecated related test has been moved to unit test
         */
        class TransportAdapter : public Transport
        {
        public:
            explicit TransportAdapter(std::shared_ptr<WriteRecorder> writeRecorder)
                : recorder(writeRecorder)
            {
            }

            void send(std::string&& message) override
            {
                recorder->record(message);
            }

        private:
            std::shared_ptr<WriteRecorder> recorder;
        };

    }


    BOOST_AUTO_TEST_CASE(httpServiceCanCreateDatabase)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/?db=test");
        BOOST_CHECK_NO_THROW(influxdb->createDatabaseIfNotExists());
    }

    BOOST_AUTO_TEST_CASE(udpServiceCanNotCreateDatabase)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("udp://localhost:8086/?db=test");
        BOOST_CHECK_THROW(influxdb->createDatabaseIfNotExists(), InfluxDBException);
    }

    BOOST_AUTO_TEST_CASE(unixServiceCanNotCreateDatabase)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("unix://localhost:8086/?db=test");
        BOOST_CHECK_THROW(influxdb->createDatabaseIfNotExists(), InfluxDBException);
    }


    BOOST_AUTO_TEST_CASE(pointWithEmptyStringFieldValueAreProperlyInserted)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/?db=test");

        BOOST_CHECK_NO_THROW(
            influxdb->write(Point{"empty_string_field"}
                                .addField("str_value", "")));
    }

    BOOST_AUTO_TEST_CASE(pointWithEmptyTagValueAreProperlyInserted)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/?db=test");

        BOOST_CHECK_NO_THROW(
            influxdb->write(Point{"empty_string_tag"}
                                .addField("str_value", "")
                                .addTag("tag", "")));
    }

    BOOST_AUTO_TEST_CASE(pointsCanBeWrittenOneByOne)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/?db=test");

        BOOST_CHECK_NO_THROW(
            influxdb->write(Point{"points_one_by_one"}
                                .addField("value", 10)
                                .addTag("host", "localhost")));

        BOOST_CHECK_NO_THROW(
            influxdb->write(Point{"points_one_by_one"}
                                .addField("value", 20)
                                .addTag("host", "localhost")));

        BOOST_CHECK_NO_THROW(
            influxdb->write(Point{"points_one_by_one"}
                                .addField("value", 200LL)
                                .addTag("host", "localhost")));

        BOOST_CHECK_NO_THROW(
            influxdb->write(Point{"points_one_by_one"}
                                .addField("str_value", "lorem ipsum")
                                .addTag("host", "localhost")));
    }

    BOOST_AUTO_TEST_CASE(pointsCanBeWrittenInVectorAsOneSingleBatch)
    {
        std::vector<Point> points = {
            Point{"points_in_vector"}.addField("value", 10).addTag("host", "localhost"),
            Point{"points_in_vector"}.addField("value", 10).addTag("host", "localhost"),
            Point{"points_in_vector"}.addField("value", 200LL).addTag("host", "localhost"),
            Point{"points_in_vector"}.addField("str_value", "lorem ipsum").addTag("host", "localhost")};

        auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/?db=test");

        BOOST_CHECK_NO_THROW(
            influxdb->write(std::move(points)));
    }

    BOOST_AUTO_TEST_CASE(pointsWrittenAsBatch)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
        influxdb->batchOf(2);

        BOOST_CHECK_NO_THROW(influxdb->write(Point{"batch_point"}.addField("value", 0).addTag("host", "localhost")));
        BOOST_CHECK_NO_THROW(influxdb->write(Point{"batch_point"}.addField("value", 1).addTag("host", "localhost")));

        BOOST_CHECK_NO_THROW(influxdb->write(Point{"batch_point"}.addField("value", 2).addTag("host", "localhost")));
        BOOST_CHECK_NO_THROW(influxdb->write(Point{"batch_point"}.addField("value", 3).addTag("host", "localhost")));
    }

    /**
     * \deprecated Moved to unit test
     */
    BOOST_AUTO_TEST_CASE(pointsWrittenAsBatchAreTransmittedInBatchesSpecified)
    {
        auto recorder = std::make_shared<WriteRecorder>();
        InfluxDB db{std::make_unique<TransportAdapter>(recorder)};
        db.batchOf(2);

        db.write(Point{"batch_point"}.addField("value", 0).addTag("host", "localhost"));
        BOOST_CHECK_EQUAL(0, recorder->count());
        db.write(Point{"batch_point"}.addField("value", 1).addTag("host", "localhost"));
        BOOST_CHECK_EQUAL(1, recorder->count());

        db.write(Point{"batch_point"}.addField("value", 2).addTag("host", "localhost"));
        BOOST_CHECK_EQUAL(1, recorder->count());
        db.write(Point{"batch_point"}.addField("value", 3).addTag("host", "localhost"));
        BOOST_CHECK_EQUAL(2, recorder->count());
    }

    BOOST_AUTO_TEST_CASE(writeWrongHostIsHandled)
    {
        auto influxdb = influxdb::InfluxDBFactory::Get("http://192.168.1.1002:8086?db=test");
        influxdb->batchOf(2);
        influxdb->write(Point{"test"}.addField("value", 10));
        BOOST_CHECK_THROW(influxdb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
    }
}
